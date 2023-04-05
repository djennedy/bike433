/**
 * Module to read data from the GPS.
 * Assumes the GPS receiver is connected to the UART Grove port on the BBG and UART2 dtb is loaded in uEnv.txt.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include "gpsController.h"

#define DEVICE_PATH "/dev/ttyS2"
#define TIME_OUT_VALUE_DS 10 // 10 deciseconds before timeout
#define GPS_BAUD_RATE B9600

#define LATITUDE_STR_LEN 9 // ddmm.mmmm from SIM28 NMEA Message Certification
#define LONGITUDE_STR_LEN 10 // dddmm.mmmm from  SIM28 NMEA Message Certification
#define DEGREE_PRECISION_LAT 2 // 2 digit precision for latitude (ddmm.mmmm)
#define DEGREE_PRECISION_LONG 3 // 3 digit precision for longitude (dddmm.mmmm)

#define LAT_VAL_INDEX 2
#define LAT_DIR_INDEX 3
#define LONG_VAL_INDEX 4
#define LONG_DIR_INDEX 5
#define INDICATOR_INDEX 6

#define NUM_NMEA_SENTENCES 4 

#define STR_MAX_LEN 1024

static const bool debugMode = false;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int serialPort;

// Function adapted from Geoffrey Hunter's article: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
// Configures the file descriptor fd for serial port read/write
// Returns 1 on error and 0 on success
static int configureSerial() {
    struct termios tty;
    
    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if(tcgetattr(serialPort, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag |= ICANON; // Enabling canonical mode (different from the article, we want to read line by line)
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = TIME_OUT_VALUE_DS;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    return 0;
}

// Functions to initialize and clean up the GPS
void Gps_init() {
    serialPort = open(DEVICE_PATH, O_RDONLY);
    
    if (serialPort < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    int configSerialErr = configureSerial();
    if (configSerialErr != 0) {
        printf("Gps_init: Initialization error!\n");
        return;
    }
}
void Gps_cleanup() {
    close(serialPort);
}

static double latDDMToDD(char* latStr, char direction) {
    char degreeStr[DEGREE_PRECISION_LAT + 1];
    char minuteStr[LATITUDE_STR_LEN - DEGREE_PRECISION_LAT + 1];

    // Populate strings
    for (int i = 0 ; i < LATITUDE_STR_LEN; i++) {
        if (i < DEGREE_PRECISION_LAT ) {
            degreeStr[i] = latStr[i];
        } else {
            minuteStr[i - DEGREE_PRECISION_LAT] = latStr[i];
        }
    }

    double degree = atof(degreeStr) + (atof(minuteStr) / 60.0);
    
    if (direction == 'S') {
        degree = -degree;
    }

    return degree;
}

static double longDDMToDD(char* longStr, char direction) {
    char degreeStr[DEGREE_PRECISION_LONG + 1];
    char minuteStr[LONGITUDE_STR_LEN - DEGREE_PRECISION_LONG + 1];

    // Populate strings
    for (int i = 0 ; i < LONGITUDE_STR_LEN; i++) {
        if (i < DEGREE_PRECISION_LONG ) {
            degreeStr[i] = longStr[i];
        } else {
            minuteStr[i - DEGREE_PRECISION_LONG] = longStr[i];
        }
    }

    double degree = atof(degreeStr) + (atof(minuteStr) / 60.0);

    if (direction == 'W') {
        degree = -degree;
    }

    return degree;
}

// Returns GPS positioning data. Data updates every 1 second +- 10 ns
Gps_values_t Gps_readData() {
    Gps_values_t result;
    result.qualityIndicator=0;

    char buf[STR_MAX_LEN];
    const char delim[2] = ",";

    memset(&buf, 0, sizeof(buf));

    while(true) {
        int numBytes;

        pthread_mutex_lock(&mutex);
        {
            numBytes = read(serialPort, &buf, sizeof(buf)); // Read one line
        }
        pthread_mutex_unlock(&mutex);

        if (numBytes < 0) {
            printf("Gps_readData: Error reading: %s", strerror(errno));
        }

        if (debugMode) {
            printf("Read %i bytes. Received message: %s\n", numBytes, buf);
        }

        char* token = strtok(buf, delim);

        // For now, we're only using $GPSGGA value, which is the first nmea sentence given by the receiver.
        if (strcmp(token, "$GPGGA") != 0){
            continue;
        } 

        char latitudeStr[LATITUDE_STR_LEN+1];
        char latitudeDir;
        char longitudeStr[LONGITUDE_STR_LEN+1];
        char longitudeDir;

        for (int tokIndex = 0; token != NULL; tokIndex++) {
            switch (tokIndex) {
                case LAT_VAL_INDEX:
                    strncpy(latitudeStr, token, LATITUDE_STR_LEN);
                    latitudeStr[LATITUDE_STR_LEN] = '\0';
                    break;
                
                case LAT_DIR_INDEX:
                    latitudeDir = token[0];
                    break;
                
                case LONG_VAL_INDEX:
                    strncpy(longitudeStr, token, LONGITUDE_STR_LEN);
                    longitudeStr[LONGITUDE_STR_LEN] = '\0';
                    break;
                
                case LONG_DIR_INDEX:
                    longitudeDir = token[0];
                    break;

                case INDICATOR_INDEX:
                    result.qualityIndicator = atoi(token);
                    break;
                
                default:
                    break;
            }

            token = strtok(NULL, delim);
        }

        result.latitudeVal = latDDMToDD(latitudeStr, latitudeDir);
        result.longitudeVal = longDDMToDD(longitudeStr, longitudeDir);
        break;
    }

    return result;
}
/**
 * Module to listen to commands sent via UDP in its own thread
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h> 
#include "networkListener.h"
#include "gpsTracker.h"

#define PORT_NUM 12345

#define STRING_MAX_LENGTH 1024
// MTU limit to ensure sent package is not broken up
// On loopback address, limit is 8192, on Ethernet it's 1464, but we want to use the smallest MTU limit as needed, which is 528 via modem
#define MODEM_MTU_LIMIT 528

#define VOLUME_CHANGE_INTERVAL 5
#define BPM_CHANGE_INTERVAL 5

typedef enum Commands_e {
    GET_LATITUDE = 0,
    GET_LONGITUDE,
    GET_LOCK_STATUS,
    LOCK_DEVICE,
    UNLOCK_DEVICE,
    GET_MOVE_STATUS,
    NUM_OF_COMMANDS
} Commands;

// Note: if the order of this array is changed, the enum will also have to be changed.
static const char* validCommandStrings[] = {
   "get_lat", "get_long", "get_lock", "lock", "unlock", "get_isMoved"
};

static bool isShutDown = false;
static pthread_t thread;

static Commands checkForCommands(char* str, int len) {
    const char* delim = " ";
    int numCommands = NUM_OF_COMMANDS;
    char* tempStr = (char*) malloc ((len + 1) * sizeof(char));
    Commands result;
    for (int i = 0; i < numCommands; i++) {
        memset(tempStr, 0, len + 1);
        char* token;
        
        strcpy(tempStr, str);
        token = strtok(tempStr, delim);
        if (strcmp(token, validCommandStrings[i]) == 0) {
            result = i;
        }
    }
    free(tempStr);
    return result;
}

// Returns a dynamically allocated string containing the reply to transmit
// getNValue is used if the command is GET_N
static char* generateReply(Commands command) {
    char* reply;
    switch(command) {
        case GET_LATITUDE: {
            Gps_values_t currGpsVal = GpsTrack_getCurrGpsVal();
            char tempString[STRING_MAX_LENGTH];
            if (currGpsVal.qualityIndicator != 0) {
                double lat = GpsTrack_getCurrGpsVal().latitudeVal;
                snprintf(tempString, STRING_MAX_LENGTH, "%f", lat);
            } else {
                strcpy(tempString, "invalid");
            }

            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        case GET_LONGITUDE: {
            Gps_values_t currGpsVal = GpsTrack_getCurrGpsVal();
            char tempString[STRING_MAX_LENGTH];
            if (currGpsVal.qualityIndicator != 0) {
                double lon = GpsTrack_getCurrGpsVal().longitudeVal;
                snprintf(tempString, STRING_MAX_LENGTH, "%f", lon);
            } else {
                strcpy(tempString, "invalid");
            }

            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        case GET_LOCK_STATUS: {
            bool isLocked = GpsTrack_isLocked();
            char* tempString = isLocked ? "is locked" : "is not locked";
            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        case LOCK_DEVICE: {
            GpsTrack_lockPosition();
            char* tempString = "locked";
            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        case UNLOCK_DEVICE: {
            GpsTrack_lockPosition();
            char* tempString = "unlocked";
            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        case GET_MOVE_STATUS: {
            bool isMoved = GpsTrack_isMoved();
            char* tempString = isMoved ? "is moved" : "is not moved";
            int stringLen = strlen(tempString);
            reply = (char*) malloc((stringLen + 1) * sizeof(char));
            strncpy(reply, tempString, stringLen);
            reply[stringLen] = '\0';
            break;
        }

        default:
            break;
    }
    return reply;
}

static void* listenToUdp() {
    // Note: networking portion is adapted from demo in class
    struct sockaddr_in sin;
    memset (&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET; // Set communictaion address to use ipv4
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Set to accept message from any address
    sin.sin_port = htons(PORT_NUM); // Set to be bound to the specified port
    
    // Creating UDP socket
    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Create socket with ipv4, UDP, any protocol

    // Bind to the port and check for errors
    if (bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
        close(socketDescriptor);
        perror("Bind error\n");
        exit(1);
    }

    while(!isShutDown) {
        // Receive the message
        struct sockaddr_in sinRemote;
        unsigned int sinLen = sizeof(sinRemote);
        char messageRx[STRING_MAX_LENGTH];
        int bytesRx = recvfrom(socketDescriptor, messageRx, STRING_MAX_LENGTH - 1, 0, (struct sockaddr *) &sinRemote, &sinLen);
        if (bytesRx == -1) {
            perror("error when receiving \n");
            exit(1);
        }
        // Remove the "\n" from the string
        messageRx[bytesRx-1] = 0;
        
        // Process the string received
        Commands commandRx= checkForCommands(messageRx, strlen(messageRx));
        char* messageTx = generateReply(commandRx);
        
        // Send a reply in chunks
        long long msgLen = strlen(messageTx);
        sinLen = sizeof(sinRemote);
        for (int i = 0; i < msgLen; i+=(MODEM_MTU_LIMIT-1)) {
            char tempStr[MODEM_MTU_LIMIT];
            strncpy(tempStr, messageTx + i, MODEM_MTU_LIMIT-1);
            tempStr[MODEM_MTU_LIMIT-1] = '\0';
            sendto(socketDescriptor, tempStr, MODEM_MTU_LIMIT, 0, (struct sockaddr *) &sinRemote, sinLen);
        }
        free(messageTx);
    }

    close(socketDescriptor);
    return NULL;
}

// Starts listening on fixed port 12345
void Net_startListening() {
    pthread_create(&thread, NULL, &listenToUdp, NULL);
}

// Stops listening and cleans up all memory used
void Net_stopListening() {
    isShutDown = true;
    pthread_join(thread, NULL);
}
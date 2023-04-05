#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>

#define GPIO_EXPORT_PATH "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH "/sys/class/gpio/unexport"

#define GPIO_JOYSTICK_UP "/sys/class/gpio/gpio26"
#define GPIO_JOYSTICK_DOWN "/sys/class/gpio/gpio46"
#define GPIO_JOYSTICK_LEFT "/sys/class/gpio/gpio65"
#define GPIO_JOYSTICK_RIGHT "/sys/class/gpio/gpio47"
#define GPIO_JOYSTICK_IN "/sys/class/gpio/gpio27"

#define GPIO_JOYSTICK_UP_NUM "26"
#define GPIO_JOYSTICK_DOWN_NUM "46"
#define GPIO_JOYSTICK_LEFT_NUM "65"
#define GPIO_JOYSTICK_RIGHT_NUM "47"
#define GPIO_JOYSTICK_IN_NUM "27"

#define PIN_JOYSTICK_UP "p8.14"
#define PIN_JOYSTICK_DOWN "p8.16"
#define PIN_JOYSTICK_LEFT "p8.18"
#define PIN_JOYSTICK_RIGHT "p8.15"
#define PIN_JOYSTICK_IN "p8.17"

/**
 * It runs a shell command and ignores the output
 * From: Brian Fraser CMPT 433 Assignment 1 Description
 *
 * @param command The command to execute.
 */
static void runCommand(char *command) {
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");

    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL) break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

/**
 * Read and return the value from the GPIO file path.
 * From: Brian Fraser CMPT 433 GPIO Guide
 *
 * @param fileName The name of the file to read from.
 * @return 1 if the value is low, 0 if high.
 */
static int readValue(char *fileName) {
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }

    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);

    // Close
    fclose(pFile);

    // Check return value
    if (buff[0] == '0') {
        return 0;
    } else if (buff[0] == '1') {
        return 1;
    } else {
        printf("ERROR: Unable to read value from file (%s)\n", fileName);
        exit(-1);
    }
}

static void configPIN() {
    runCommand(
        "config-pin"
        " " PIN_JOYSTICK_UP
        " "
        "gpio");
    runCommand(
        "config-pin"
        " " PIN_JOYSTICK_DOWN
        " "
        "gpio");
    runCommand(
        "config-pin"
        " " PIN_JOYSTICK_LEFT
        " "
        "gpio");
    runCommand(
        "config-pin"
        " " PIN_JOYSTICK_RIGHT
        " "
        "gpio");
    runCommand(
        "config-pin"
        " " PIN_JOYSTICK_IN
        " "
        "gpio");
}

static void unexportPin() {
    runCommand("echo " GPIO_JOYSTICK_UP_NUM " > " GPIO_UNEXPORT_PATH);
    runCommand("echo " GPIO_JOYSTICK_DOWN_NUM " > " GPIO_UNEXPORT_PATH);
    runCommand("echo " GPIO_JOYSTICK_LEFT_NUM " > " GPIO_UNEXPORT_PATH);
    runCommand("echo " GPIO_JOYSTICK_RIGHT_NUM " > " GPIO_UNEXPORT_PATH);
    runCommand("echo " GPIO_JOYSTICK_IN_NUM " > " GPIO_UNEXPORT_PATH);
}

static void setPinToInput() {
    runCommand(
        "echo in > "
        "" GPIO_JOYSTICK_UP
        ""
        "/direction");
    runCommand(
        "echo in > "
        "" GPIO_JOYSTICK_DOWN
        ""
        "/direction");
    runCommand(
        "echo in > "
        "" GPIO_JOYSTICK_LEFT
        ""
        "/direction");
    runCommand(
        "echo in > "
        "" GPIO_JOYSTICK_RIGHT
        ""
        "/direction");
    runCommand(
        "echo in > "
        "" GPIO_JOYSTICK_IN
        ""
        "/direction");
}

void GPIO_init() {
    configPIN();
    setPinToInput();
}

void GPIO_cleanup() {
    unexportPin();
}

/**
 * It reads the values of the four GPIO pins from the joystick
 * and returns the direction that the joystick is currently pointing
 *
 * @return The direction enum of the joystick (UP, DOWN, LEFT, RIGHT, NONE)
 */
JoystickDirection GPIO_getJoystickDirection() {
    int up = readValue("" GPIO_JOYSTICK_UP
                       ""
                       "/value");
    int down = readValue("" GPIO_JOYSTICK_DOWN
                         ""
                         "/value");
    int left = readValue("" GPIO_JOYSTICK_LEFT
                         ""
                         "/value");
    int right = readValue("" GPIO_JOYSTICK_RIGHT
                          ""
                          "/value");
    int in = readValue("" GPIO_JOYSTICK_IN
                       ""
                       "/value");

    if (up == 0) {
        return UP;
    } else if (down == 0) {
        return DOWN;
    } else if (left == 0) {
        return LEFT;
    } else if (right == 0) {
        return RIGHT;
    } else if (in == 0) {
        return IN;
    } else {
        return NONE;
    }
}

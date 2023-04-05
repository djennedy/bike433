/**
 * Module to observe the watchdog. Watchdog times out after 30 seconds. Each thread needs to hit the watchdog at least once in order to hit the watchdog
 * Code adapted from in-class watchdog examples
*/

#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "watchdog.h"

#define WATCHDOG_PATH "/dev/watchdog"
#define TIMEOUT_S 45

typedef enum {
    GPS_WATCHDOG = 0,
    JOYSTICK_WATCHDOG,
    BUZZER_WATCHDOG,
    MAIN_WATCHDOG,
    NUM_OF_WATCHDOGS
} Watchdogs_e;

static bool watchdogHits[NUM_OF_WATCHDOGS];

static bool isShutDown = false;
static pthread_t thread;

static void dieOnError(_Bool successCondition, char *message)
{
	if (!successCondition) {
		fprintf(stderr, "ERROR: %s\n", message);
		fprintf(stderr, "Error string: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static bool allWatchdogsHit() {
    for (int i = 0; i < NUM_OF_WATCHDOGS; i++) {
        if (!watchdogHits[i]) {
            return false;
        }
    }
    return true;
}

static void setWatchdogsFalse() {
    for (int i = 0; i < NUM_OF_WATCHDOGS; i++) {
        watchdogHits[i] = false;
    }
}

static void* startWatchdogs() {
    while (!isShutDown) {
        if (allWatchdogsHit()) {
            int fd = open(WATCHDOG_PATH, O_RDWR);
	        dieOnError(fd != -1, "Unable to open WD.");

            write(fd, "w", 1);

            close(fd);
            setWatchdogsFalse();
        }
    }
    return NULL;
}

// Initializing and cleanup functions
void Watchdog_init() {
    setWatchdogsFalse();

    int fd = open(WATCHDOG_PATH, O_RDWR);
    dieOnError(fd != -1, "Unable to open WD.");

    int timeout = TIMEOUT_S;
    int result = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    dieOnError(result == 0, "Unable to set watchdog timout.");

    close(fd);

    pthread_create(&thread, NULL, &startWatchdogs, NULL);
}

void Watchdog_cleanup() {
    int fd = open(WATCHDOG_PATH, O_RDWR);
    dieOnError(fd != -1, "Unable to open WD.");

    // Turn off watchdog
    write(fd, "V", 1);

    close(fd);

    isShutDown = true;
    pthread_join(thread, NULL);
}

// Hits the watchdog, to be called by the gpsTracker thread
void Watchdog_gpsTrackerHit() {
    watchdogHits[GPS_WATCHDOG] = true;
}

// Hits the watchdog, to be called by the joystick thread
void Watchdog_joystickHit() {
    watchdogHits[JOYSTICK_WATCHDOG] = true;
}

// Hits the watchdog, to be called by the buzzer thread
void Watchdog_buzzerHit() {
    watchdogHits[BUZZER_WATCHDOG] = true;
}

// Hits the watchdog, to be called by the main thread
void Watchdog_mainThreadHit() {
    watchdogHits[MAIN_WATCHDOG] = true;
}
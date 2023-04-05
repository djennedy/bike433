#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gpsController.h"
#include "gpsTracker.h"
#include "networkListener.h"
#include "buzzer.h"
#include "joystick.h"
#include "shutdown.h"

void initialize() {
    Gps_init();
    Joystick_init();
    Buzzer_init();
    GpsTrack_startTracking();
    Net_startListening();
}

void cleanup() {
    Net_stopListening();
    GpsTrack_stopTracking();
    Buzzer_cleanup();
    Joystick_cleanup();
    Gps_cleanup();
}

int main() {
    initialize();

    Shutdown_waitForShutdown();

    cleanup();
};
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gpsController.h"
#include "gpsTracker.h"
#include "networkListener.h"

#include "gpsTests.h"

void initialize() {
    Gps_init();
    GpsTrack_startTracking();
    Net_startListening();
}

void cleanup() {
    Net_stopListening();
    GpsTrack_stopTracking();
    Gps_cleanup();
}

int main() {
    initialize();

    GpsTest_testGpsController();

    cleanup();
};
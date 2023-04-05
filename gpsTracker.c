/**
 * Module to continuously track GPS on it's own thread. Can also "lock" a position to notify if the device has moved more than a 3 meter square radius.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include "gpsController.h"
#include "gpsTracker.h"
#include "buzzer.h"

#define MY_PI (3.14159265358979323846264338327950288) // Value copied from https://c-for-dummies.com/blog/?p=3015

#define DIST_THRESHOLD_m 5

#define DEGREE_METER_ESTIMATE 111111

static bool isShutDown = false;
static pthread_t thread;

static bool isLocked = false;
static bool isMoved = false;
static Gps_values_t lockedGpsVal;

static Gps_values_t currGpsVal;

static double getDistanceBetweenLats(double lat1, double lat2) {
    // Formula used is a very rough approximate, taken from username whuber's comment on  https://gis.stackexchange.com/questions/2951/algorithm-for-offsetting-a-latitude-longitude-by-some-amount-of-meters
    // Using an exact formula, such as haversine, will introduce rounding errors because the difference between our values are small

    double lat1Meters = lat1 * DEGREE_METER_ESTIMATE;
    double lat2Meters = lat2 * DEGREE_METER_ESTIMATE;
    return fabs(lat1Meters - lat2Meters);
}

static double getDistanceBetweenLons(double lat1, double lat2) {
    // Formula used is a very rough approximate, taken from username whuber's comment on  https://gis.stackexchange.com/questions/2951/algorithm-for-offsetting-a-latitude-longitude-by-some-amount-of-meters
    // Using an exact formula, such as haversine, will introduce rounding errors because the difference between our values are small

    double lat1Radians = lat1 * (MY_PI/180);
    double lat2Radians = lat2 * (MY_PI/180);
    double lon1Meters = cos(lat1Radians) * DEGREE_METER_ESTIMATE;
    double lon2Meters = cos(lat2Radians) * DEGREE_METER_ESTIMATE;
    return fabs(lon1Meters - lon2Meters);
}

static void* trackGps() {
    while(!isShutDown) {
        currGpsVal = Gps_readData();

        // If the gps value is invalid, continue
        if (currGpsVal.qualityIndicator == 0) {
            continue;
        }

        if (isLocked) {
            double latDist = getDistanceBetweenLats(currGpsVal.latitudeVal, lockedGpsVal.latitudeVal);
            double lonDist = getDistanceBetweenLons(currGpsVal.latitudeVal, lockedGpsVal.latitudeVal);
            printf("Distance lat: %f, Distance long: %f\n", latDist, lonDist);

            if ((latDist > DIST_THRESHOLD_m || lonDist > DIST_THRESHOLD_m) && !isMoved) {
                printf("Distance lat: %f, Distance long: %f, Turning on Buzzer\n", latDist, lonDist);
                isMoved = true;
                Buzzer_alarmOn();
            }
        }
    }

    return NULL;
}

// Functions to start and stop tracking
void GpsTrack_startTracking() {
    pthread_create(&thread, NULL, &trackGps, NULL);
}
void GpsTrack_stopTracking() {
    isShutDown = true;
    pthread_join(thread, NULL);
}

// Locks the current positon, sending an alert if the device has gone more than 5 meters
void GpsTrack_lockPosition() {
    isLocked = true;
    lockedGpsVal = currGpsVal;
    isMoved = false;
    Buzzer_alarmOff();
}

// Unlocks the current positon
void GpsTrack_unlockPosition() {
    isLocked = false;
    isMoved = false; // Resets isMoved if gps is unlocked
    Buzzer_alarmOff();
}

// Returns true if the gps is in lock state but is mov3ed more than 5 meters
bool GpsTrack_isMoved() {
    return isMoved;
}

bool GpsTrack_isLocked() {
    return isLocked;
}

Gps_values_t GpsTrack_getCurrGpsVal() {
    return currGpsVal;
}
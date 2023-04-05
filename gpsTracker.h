#ifndef GPSTRACKER_H
#define GPSTRACKER_H

/**
 * Module to continuously track GPS on it's own thread. Can also "lock" a position to notify if the device has moved more than a 3 meter square radius.
*/

#include "gpsController.h"

// Functions to start and stop tracking
void GpsTrack_startTracking();
void GpsTrack_stopTracking();

// Locks the current positon
void GpsTrack_lockPosition();

// Unlocks the current positon
void GpsTrack_unlockPosition();

// Returns true if the gps is locked
bool GpsTrack_isLocked();

// Returns true if the gps is in lock state but is moved more than 3 meters
bool GpsTrack_isMoved();

// Returns the current gps value
Gps_values_t GpsTrack_getCurrGpsVal();

#endif /* GPSTRACKER_H */

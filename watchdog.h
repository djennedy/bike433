#ifndef WATCHDOG_H
#define WATCHDOG_H

/**
 * Module to observe the watchdog. Watchdog times out after 30 seconds. Each thread needs to hit the watchdog at least once in order to hit the watchdog (except net listener)
 * Code adapted from in-class watchdog examples
*/

// Initializing and cleanup functions
void Watchdog_init();
void Watchdog_cleanup();

// Hits the watchdog, to be called by the gpsTracker thread
void Watchdog_gpsTrackerHit();

// Hits the watchdog, to be called by the main thread
void Watchdog_mainThreadHit();

// Hits the watchdog, to be called by the joystick thread
void Watchdog_joystickHit();

// Hits the watchdog, to be called by the buzzer thread
void Watchdog_buzzerHit();


#endif /* WATCHDOG_H */

#ifndef GPSCONTROLLER_H
#define GPSCONTROLLER_H

/**
 * Module to read data from the GPS. 
 * Assumes the GPS receiver is connected to the UART Grove port on the BBG and UART2 dtb is loaded in uEnv.txt.
*/

typedef struct {
    // GPS quality indicator. If it's 0 the whole value is invalid.
    // 0: Invalid
    // 1: GPS SPS Mode, fix valie
    // 2: Differential GPS, SPS Mode, fix valid
    // 6: Dead Reckoning Mode, fix valid
    int qualityIndicator;

    double latitudeVal;
    double longitudeVal;
} Gps_values_t;


// Functions to initialize and clean up the GPS
void Gps_init();
void Gps_cleanup();

// Returns GPS positioning data. Data updates every 1 second +- 10 ns
Gps_values_t Gps_readData();

#endif /* GPSCONTROLLER_H */

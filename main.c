#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gpsController.h"


void initialize() {
    Gps_init();
}

void cleanup() {
    Gps_cleanup();
}

int main() {
    initialize();

    // Wait for shutdown

    cleanup();
};
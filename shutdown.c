/**
 * Module to wait until shutdown condition is fullfilled
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shutdown.h"
#include "watchdog.h"

static bool isShutDown = false;

// Wait for shutdown to be triggered
void Shutdown_waitForShutdown() {
    while (!isShutDown){
        Watchdog_mainThreadHit();
    }
}

// Triggers shutdown for all modules, including itself
void Shutdown_triggerShutdown() {
    isShutDown = true;
}
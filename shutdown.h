#ifndef SHUTDOWN_H
#define SHUTDOWN_H

/**
 * Module to wait until shutdown condition is fullfilled
*/

// Wait for shutdown to be triggered
void Shutdown_waitForShutdown();

// Triggers shutdown for all modules, including itself
void Shutdown_triggerShutdown();

#endif /* SHUTDOWN_H */

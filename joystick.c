#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "buzzer.h"
#include "gpio.h"
#include "gpsTracker.h"
#include "timer.h"

typedef enum {
    IDLE,
    PRESS_1_LOCK,
    PRESS_2_LOCK,
    PRESS_3_LOCK,
    PRESS_1_UNLOCK,
    PRESS_2_UNLOCK,
    PRESS_3_UNLOCK,
    LOCKED,
    UNLOCKED
} JoystickState;

static pthread_t joystickThreadID;
static bool joystickRunning = false;
static JoystickState joystickState = IDLE;

void* joystickThread(void* args);
void Joystick_init(void);
void Joystick_cleanup(void);

void* joystickThread(void* args) {
    while (joystickRunning) {
        JoystickDirection direction = GPIO_getJoystickDirection();

        if (direction != NONE) {
            Buzzer_quickBuzz();
            switch (joystickState) {
                case IDLE:
                    if (direction == DOWN) {
                        joystickState = PRESS_1_LOCK;
                    } else if (direction == LEFT) {
                        joystickState = PRESS_1_UNLOCK;
                    }
                    break;

                case PRESS_1_LOCK:
                    if (direction == UP) {
                        joystickState = PRESS_2_LOCK;
                    } else {
                        joystickState = IDLE;
                    }
                    break;

                case PRESS_2_LOCK:
                    if (direction == DOWN) {
                        joystickState = PRESS_3_LOCK;
                    } else {
                        joystickState = IDLE;
                    }
                    break;

                case PRESS_3_LOCK:
                    if (direction == IN) {
                        GpsTrack_lockPosition();
                        printf("Locked\n");
                    }
                    joystickState = IDLE;
                    break;

                case PRESS_1_UNLOCK:
                    if (direction == RIGHT) {
                        joystickState = PRESS_2_UNLOCK;
                    } else {
                        joystickState = IDLE;
                    }
                    break;

                case PRESS_2_UNLOCK:
                    if (direction == LEFT) {
                        joystickState = PRESS_3_UNLOCK;
                    } else {
                        joystickState = IDLE;
                    }
                    break;

                case PRESS_3_UNLOCK:
                    if (direction == IN) {
                        GpsTrack_unlockPosition();
                        printf("Unlocked\n");
                    }
                    joystickState = IDLE;
                    break;
                default:
                    joystickState = IDLE;
                    break;
            }
        }
        Timer_sleepForMs(100);
    }
    return NULL;
}

void Joystick_init(void) {
    GPIO_init();
    joystickRunning = true;
    pthread_create(&joystickThreadID, NULL, joystickThread, NULL);
}

void Joystick_cleanup(void) {
    joystickRunning = false;
    pthread_join(joystickThreadID, NULL);
}
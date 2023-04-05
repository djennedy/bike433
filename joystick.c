#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "buzzer.h"
#include "gpio.h"
#include "gpsTracker.h"
#include "timer.h"

typedef enum {
    IDLE,
    PRESS_1,
    PRESS_2,
    PRESS_3,
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
        bool isLocked = GpsTrack_isLocked();

        if (direction != NONE) {
            Buzzer_quickBuzz();
            switch (joystickState) {
                case IDLE:
                    if ((direction == DOWN && !isLocked) ||
                        (direction == LEFT && isLocked)) {
                        joystickState = PRESS_1;
                    }
                    break;
                case PRESS_1:
                    if ((direction == UP && !isLocked) ||
                        (direction == RIGHT && isLocked)) {
                        joystickState = PRESS_2;
                    } else {
                        joystickState = IDLE;
                    }
                    break;
                case PRESS_2:
                    if ((direction == DOWN && !isLocked) ||
                        (direction == LEFT && isLocked)) {
                        joystickState = PRESS_3;
                    } else {
                        joystickState = IDLE;
                    }
                    break;
                case PRESS_3:
                    if (direction == IN) {
                        // TODO: Change so we don't have to use toggle
                        if (isLocked) {
                            GpsTrack_lockPosition();
                        } else {
                            GpsTrack_unlockPosition();
                        }
                    }
                    joystickState = IDLE;
                    break;
                default:
                    joystickState = IDLE;
                    break;
            }
        }
        printf("Direction: %d, State: %d\n", direction, joystickState);
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
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "buzzer.h"
#include "gpio.h"
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
static bool isLocked = false;
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
                        joystickState = PRESS_1;
                    }
                    break;
                case PRESS_1:
                    if (direction == UP) {
                        joystickState = PRESS_2;
                    } else {
                        joystickState = IDLE;
                    }
                    break;
                case PRESS_2:
                    if (direction == DOWN) {
                        joystickState = PRESS_3;
                    } else {
                        joystickState = IDLE;
                    }
                    break;
                case PRESS_3:
                    if (direction == IN) {
                        isLocked = !isLocked;
                        printf("Locked or Unlocked! %d\n", isLocked);
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
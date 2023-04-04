/*
 * The GPIO module is used to initialize the
 * GPIO and process the user input from the
 * joystick. This module contains the path
 * to the respective GPIO modules and the enum
 * representing the direction of the joystick input.
 */
#ifndef GPIO_H  // Inclusion guard.
#define GPIO_H  // Inclusion guard.

typedef enum JoystickDirection {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    IN
} JoystickDirection;

void GPIO_init();
JoystickDirection GPIO_getJoystickDirection();

#endif
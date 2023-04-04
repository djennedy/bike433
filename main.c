#include <stdio.h>
#include <unistd.h>

#include "buzzer.h"
#include "joystick.h"

int main(void) {
    Buzzer_init();
    Joystick_init();
    

    while(1){
        sleep(1);
    }

    // Buzzer_quickBuzz();
    // sleep(1);
    // Buzzer_quickBuzz();
    // sleep(1);


    // Buzzer_alarmOn();
    // sleep(5);
    // Buzzer_alarmOff();

    Joystick_cleanup();
    Buzzer_cleanup();
    return 0;
}
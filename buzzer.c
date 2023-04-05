#include "buzzer.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "command.h"
#include "timer.h"

//For Click Board Buzzer
#define BUZZER_PIN "P8_13"
#define PWM_DIR "/dev/bone/pwm/2/b/"

//For Zen Cape Buzzer
// #define BUZZER_PIN "P9_22"
// #define PWM_DIR "/dev/bone/pwm/0/a/"

#define BUZZER_LOW_PERIOD "1000000"
#define BUZZER_LOW_DUTY_CYCLE "500000"

#define ON "1"
#define OFF "0"

static bool buzzerOn = false;
static bool buzzerRunning = false;
static bool buzzerAlarmMode = false;
pthread_t buzzerThreadID;
static pthread_mutex_t alarmMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t buzzerMutex = PTHREAD_MUTEX_INITIALIZER;

void config_pwm(void) {
    Command_runCommand("config-pin " BUZZER_PIN " pwm");
    buzzerRunning = true;
}

void setTone(void) {
    Command_runCommand("echo " BUZZER_LOW_PERIOD " > " PWM_DIR "period");
    Command_runCommand("echo " BUZZER_LOW_DUTY_CYCLE " > " PWM_DIR
                       "duty_cycle");
}

void turnOn(void) { Command_runCommand("echo " ON " > " PWM_DIR "enable"); }

void turnOff(void) { Command_runCommand("echo " OFF " > " PWM_DIR "enable"); }

void *buzzerThread(void *arg) {
    while (buzzerRunning) {
        if (buzzerOn || (buzzerAlarmMode && Timer_getTimeInMs() % 1000 < 250)) {
            turnOn();
        } else {
            turnOff();
        }
    }
    return NULL;
}

void Buzzer_quickBuzz(void) {
    pthread_mutex_lock(&buzzerMutex);
    {
        buzzerOn = true;
        Timer_sleepForMs(50);
        buzzerOn = false;
        Timer_sleepForMs(50);
    }
    pthread_mutex_unlock(&buzzerMutex);
}

void Buzzer_alarmOn(void) {
    pthread_mutex_lock(&alarmMutex);
    { buzzerAlarmMode = true; }
    pthread_mutex_unlock(&alarmMutex);
}

void Buzzer_alarmOff(void) {
    buzzerAlarmMode = false;
    pthread_mutex_lock(&alarmMutex);
    { buzzerAlarmMode = false; }
    pthread_mutex_unlock(&alarmMutex);
}

void Buzzer_init(void) {
    config_pwm();
    setTone();
    pthread_create(&buzzerThreadID, NULL, buzzerThread, NULL);
}

void Buzzer_cleanup(void) {
    buzzerOn = false;
    buzzerRunning = false;
    buzzerAlarmMode = false;
    pthread_join(buzzerThreadID, NULL);
}

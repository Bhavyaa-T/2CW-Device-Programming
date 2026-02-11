#include "mbed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
InterruptIn button(USER_BUTTON);

const int N = 3;
const int M = 5;
int led_cycle[N]={1,2,3};


int sequence[M];
volatile int t;
volatile int i = 0;
volatile bool seq_done = false;

Timeout button_debounce_timeout;
float debounce_time_interval = 0.3;

void select_led(int l)
{
        if (l==1) {
                led1 = true;
                led2 = false;
                led3 = false;
        }
        else if (l==2) {
                led1 = false;
                led2 = true;
                led3 = false;
        }
        else if (l==3) {
                led1 = false;
                led2 = false;
                led3 = true;
        }
}


void set_sequence(int* a, int n) {
    if (i < M) {
    *(a + i) = n;
    i = i + 1;
        if (i == M) {
            seq_done = true;
            button.rise(NULL);
        }
    }

}

void onButtonStopDebouncing(void);

void onButtonPress() {
    set_sequence(sequence, led_cycle[t]);
    led1 = true;
    led2 = true;
    led3 = true;
    button.rise(NULL);
    button_debounce_timeout.attach(onButtonStopDebouncing, debounce_time_interval);
}

void onButtonStopDebouncing(void) {
    if (i < M) {
    button.rise(onButtonPress);
    }
}

int l = 0;

int main() {
    t=0;
    button.rise(onButtonPress);
    while(true) {
        if (!seq_done) {
                select_led(led_cycle[t]);
                wait(1);
                t=(t+1)%N;
        }
        else {
            select_led(sequence[l]);
            wait(0.2);
            led1 = false;
            led2 = false;
            led3 = false;
            wait(0.2); // turning on and off makes the LEDs flash
            l=(l+1)%M;
        }
        }
}
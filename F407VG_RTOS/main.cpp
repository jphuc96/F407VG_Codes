#include "mbed.h"
#include "rtos.h"
 
DigitalOut led1(PD_12);
DigitalOut led2(PD_13);
DigitalOut led3(PD_14);
DigitalOut led4(PD_15);

void led1_thread(void const *args);
void led2_thread(void const *args);
void led3_thread(void const *args);
void led4_thread(void const *args);

int main() {
    Thread thread1(led1_thread);
    Thread thread2(led2_thread);
    Thread thread3(led3_thread);
    Thread thread4(led4_thread);
    while (true) {
        //led1 = !led1;
        //Thread::wait(250);
    }
}

void led1_thread(void const *args) {
    while (true) {
        led1 = !led1;
        Thread::wait(250);
    }
}

void led2_thread(void const *args) {
    while (true) {
        led2 = !led2;
        Thread::wait(500);
    }
}

void led3_thread(void const *args) {
    while (true) {
        led3 = !led3;
        Thread::wait(1000);
    }
}

void led4_thread(void const *args) {
    while (true) {
        led4 = !led4;
        Thread::wait(2000);
    }
}
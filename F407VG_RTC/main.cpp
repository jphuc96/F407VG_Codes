#include "mbed.h"
#include "rtc_api_hal.h"
Serial pc(PA_9,PA_10,115200);
time_t t;
int main() {

    rtc_init();
    //rtc_write(1519531440);
    set_time(rtc_read());
    
    while (true) {
        t = time(NULL);
        pc.printf("Time now: %s", ctime(&t));
        wait(1);
    }
}

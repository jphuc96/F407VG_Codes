#include "mbed.h"
#include "sdio.h"
#include "gpio.h"

int main()
{
    MX_GPIO_Init();
    MX_SDIO_SD_Init();
}
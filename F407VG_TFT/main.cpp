#include "mbed.h"
#include "MyTouch.h"
#include "SPI_TFT_ILI9341.h"
#include "Arial12x12.h"

#define TFT_MOSI    PB_5
#define TFT_MISO    PB_4
#define TFT_SCLK    PA_5
#define TFT_CS      PA_4
#define TFT_DC      PB_6
#define TFT_RS      PB_7


#define TOUCH_MOSI  PB_15 
#define TOUCH_MISO  PB_14
#define TOUCH_SCLK  PB_10
#define TOUCH_CS    PB_9
#define TOUCH_IRQ   PB_8

void mytouch_irq_handler(void);
volatile unsigned int x, y ;

Serial pc(PA_9,PA_10,115200);
MYTOUCH MyTouch(TOUCH_MOSI,TOUCH_MISO,TOUCH_SCLK,TOUCH_CS,TOUCH_IRQ, USE_12BITS, &mytouch_irq_handler);
SPI_TFT_ILI9341 tft(TFT_MOSI,TFT_MISO,TFT_SCLK,TFT_CS,TFT_RS,TFT_DC,"TFT");



void mytouch_irq_handler(void)
{   
    float xf, yf ;
    x = MyTouch.TP_GetX();
    y = MyTouch.TP_GetY();
    xf = x ;
    yf = y ;
    xf = (xf/1024)*320 ;
    yf = (yf/1024)*240 ;
    x = 320-(int)xf ; //for orientation 1
    y = 240-(int)yf ; //for orientation 1
    tft.locate(0,0);
    tft.printf("X: %d   Y: %d    \r\n",x,y);
    //tft.pixel(x,y,Red);
    tft.fillcircle(x,y,5,Green);
}

int main()
{
    tft.set_orientation(1);
    tft.background(Black);
    tft.foreground(White);
    tft.cls();                // clear the screen
    tft.set_font((unsigned char*) Arial12x12);  // select the font
    tft.printf("Hello");

    while(1)
    {
        wait_ms(100);
    }

}
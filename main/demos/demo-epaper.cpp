#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Should match with your epaper module, size
//#include "goodisplay/gdey0154d67.h"
//#include <gdep015OC1.h>  // 1.54 old version
//#include <gdeh0154d67.h>
//#include "wave12i48.h"
//#include <gdew042t2Grays.h>  // Tested correctly 06.06.20
//#include <gdew0583t7.h>
//#include <gdew0213i5f.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>
//#include "gdem029E97.h"
#include "gdew075T7.h"
// New GOODISPLAY models
//#include "color/gdew0583z83.h"
//#include "goodisplay/gdeq037T31.h"
// Color
#define IS_COLOR_EPD 0

// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
/* Epd4Spi io;
Wave12I48 display(io); */
//#include <goodisplay/gdey042T81.h>

EpdSpi io;
Gdew075T7 display(io);
//Gdep015OC1 display(io);
//Gdem029E97 display(io);
//gdey0154d67 display(io);

// Enable Power on Display
#define GPIO_DISPLAY_POWER_ON GPIO_NUM_4
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M100pt7b.h>
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h>

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }


void demo(uint16_t bkcolor, uint16_t fgcolor)
{
   display.fillScreen(bkcolor);
   display.setTextColor(fgcolor);
   display.setCursor(10, 40);
   display.setFont(&Ubuntu_M24pt8b);
   display.println("CalEPD display test\n");
   // Print all character from an Adafruit Font
   if (true)
   {
      for (int i = 40; i <= 126; i++)
      {
         display.write(i); // Needs to be >32 (first character definition)
      }
   }
}

uint16_t rectW = 0;

void draw_content(uint8_t rotation){
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
    

   uint16_t foregroundColor = EPD_BLACK;
   // Make some rectangles showing the different colors or grays
   if (display.colors_supported>1) {
      printf("display.colors_supported:%d\n", display.colors_supported);
      foregroundColor = 0xF700; // RED
   }

   rectW = display.width();
   uint16_t rectH = display.height();
   if (display.getRotation() %2 == 0) {
     rectW = display.height();
     rectH = display.width();
   }

   rectW = display.width()/4; // For 11 is 37.
   display.fillScreen(EPD_WHITE);
   display.setFont(&Ubuntu_M24pt8b);
   uint16_t firstBlock = display.width()/8;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_WHITE);
   display.fillRect(rectW*2,1,rectW,firstBlock,foregroundColor);
   display.fillRect(rectW*3,1,rectW-2,firstBlock,EPD_WHITE); 

   display.setFont(&Ubuntu_M24pt8b);
   display.setTextColor(foregroundColor);
   display.setCursor(4, 180);
   display.println("HELLO");

   display.setTextColor(foregroundColor);
   display.println("TWITTER");

   // Second row, comment if your epaper is too small
   /* display.fillRect(    1,firstBlock,rectW,firstBlock,EPD_BLACK);
   display.fillRect(rectW,firstBlock,rectW,firstBlock,EPD_RED);
   if (display.colors_supported>1) {
      display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_BLACK);
   } else {
      display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_WHITE);
   } 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,EPD_RED); */

   display.setCursor(4, 34);
   display.setTextColor(EPD_WHITE);
   display.println("OK");

   display.setFont(&Ubuntu_M100pt7b);
   display.setCursor(1, 390);
   display.setTextColor(EPD_BLACK);
   display.println("GOOD");
   display.update();
   delay(1000);

}

void app_main(void)
{
   // This is just in case you power your epaper VCC with a GPIO
   gpio_set_direction(GPIO_DISPLAY_POWER_ON, GPIO_MODE_OUTPUT);
   gpio_set_level(GPIO_DISPLAY_POWER_ON, 1);
   delay(100);
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   display.init();
   draw_content(1);

   delay(2500);
   display.fillScreen(EPD_WHITE);

   display.fillCircle(rectW, display.height()/2,90, EPD_BLACK);
   display.drawCircle(rectW*2, display.height()/2,90, EPD_BLACK);
   display.fillCircle(rectW*3, display.height()/2,90, EPD_BLACK);
   
   display.update();
   delay(2500);

   display.fillScreen(EPD_WHITE);

   
   #ifndef IS_COLOR_EPD
      display.setMonoMode(true);
   #endif

   printf("EPD width: %d height: %d\n\n", display.width(), display.height());
   
   // Partial update test. Loading bar:
   #ifndef IS_COLOR_EPD
   auto boxw = 10;
   auto boxh = 10;
   for (int a = 1; a<92 ; a+=10) {
      display.fillRect(a, 40, boxw, boxh, EPD_BLACK);
      display.updateWindow(a,40, boxw, boxh);
      delay(500);
   }

   delay(1000);
   display.fillScreen(EPD_WHITE);

   
   display.setMonoMode(false);
   #endif

   delay(1500);
   display.fillScreen(EPD_WHITE);
   printf("display: We are done with the demo\n");
   display.update();

}

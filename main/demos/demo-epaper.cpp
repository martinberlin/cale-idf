#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Should match with your epaper module, size
//#include <gdep015OC1.h>  // 1.54 old version
//#include <gdeh0154d67.h>
//#include "wave12i48.h"
//#include <gdew042t2.h>  // Tested correctly 06.06.20
//#include <gdew0583t7.h>
//#include <gdew0213i5f.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>
//#include "goodisplay/gdey0213b74.h"
#include "goodisplay/gdey0154d67.h"
// Color
//#include <gdew0583z21.h>
//#include <gdeh042Z96.h>
//#include <gdeh042Z21.h>
// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
/* Epd4Spi io;
Wave12I48 display(io); */

// Single SPI EPD
EpdSpi io;
//Gdew075T7 display(io);
//Gdep015OC1 display(io);
gdey0154d67 display(io);

// Enable on HIGH 5V boost converter
#define GPIO_ENABLE_5V GPIO_NUM_38

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
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
   display.setFont(&Ubuntu_M12pt8b);
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

void draw_content(uint8_t rotation){
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   uint16_t foregroundColor = EPD_BLACK;
   // Make some rectangles showing the different colors or grays
   if (display.colors_supported>1) {
      printf("display.colors_supported:%d\n", display.colors_supported);
      foregroundColor = EPD_RED;
   }
   display.setTextColor(EPD_BLACK);
   display.setFont(&Ubuntu_M12pt8b);
   display.fillRect(1,1,11,11,EPD_BLACK);
   display.fillRect(1,display.width()-10,11,11,EPD_BLACK);
   display.fillRect(display.height()-10,0,10,10,EPD_BLACK);
   display.fillRect(display.height()-10,display.width()-10,10,10,EPD_BLACK);
   display.setCursor(10, display.height()/2-12);
   display.print("corners");
   display.update();
   delay(2000);

   display.fillScreen(EPD_WHITE);
   display.setCursor(4, 34);
   display.setFont(&Ubuntu_M24pt8b);
   display.println("HELLO");
   display.println("TWITTER");
   display.update();
   delay(2000);

   display.fillScreen(EPD_WHITE);
   display.setFont(&Ubuntu_M12pt8b);
   uint16_t firstBlock = display.width()/8;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_WHITE);
   display.fillRect(rectW*2,1,rectW,firstBlock,foregroundColor); 
   display.fillRect(rectW*3,1,rectW-2,firstBlock,EPD_WHITE); 
   // Second row, comment if your epaper is too small
   display.fillRect(    1,firstBlock,rectW,firstBlock,EPD_WHITE);
   display.fillRect(rectW,firstBlock,rectW,firstBlock,foregroundColor);
   if (display.colors_supported>1) {
      display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_BLACK);
   } else {
      display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_WHITE);
   } 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,foregroundColor);

   display.setCursor(4, 20);
   display.setTextColor(EPD_WHITE);
   display.println("OK");

   display.setCursor(1, 90);
   display.setTextColor(EPD_BLACK);
   display.println("GOOD\nDISPLAY");
   // DRAW Marker line
   //display.fillRect(1, 97, 20, 2, EPD_BLACK);
   //display.fillRect(1, 159, 20, 2, EPD_BLACK);
   display.update();

   delay(1000);
   display.fillScreen(EPD_WHITE);
   // test partial update
   /* display.setCursor(1, 140);
   //display.println("XXX");
   display.fillCircle(30,120,20, EPD_BLACK);
   // Doing a    1, 100 prints again DISPLAY
   display.updateWindow(1,100, 120, 60);
   delay(2000); */
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // 5V is not enought for Front light if there are Leds in serie
   //gpio_set_level(GPIO_ENABLE_5V, 1);

   // Test Epd class
   display.init(false);
   
   //display.setMonoMode(true); // false = 4 gray mode without partial update
   
   display.setRotation(0);
   draw_content(display.getRotation());
   display.spi_optimized = false;
   display.setRotation(1);
   draw_content(display.getRotation()); 

   /*  
    display.setRotation(2);
   draw_content(display.getRotation());
   display.setRotation(3);
   draw_content(display.getRotation()); */

   printf("display: We are done with the demo");
}

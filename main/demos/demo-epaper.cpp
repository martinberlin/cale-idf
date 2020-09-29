#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"
// Should match with your epaper module, size
//#include "wave12i48.h"
//#include <gdew042t2.h>  // Tested correctly 06.06.20
//#include <gdew0583t7.h>
//#include <gdew075T7.h>
#include <gdew027w3.h>
//#include <gdeh0213b73.h>
// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);
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
Gdew027w3 display(io);
//Gdeh0213b73 display(io); // Does not work correctly yet - moved to /fix

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>

extern "C"
{
   void app_main();
}


void demoPartialUpdate(uint16_t bkcolor, uint16_t fgcolor, uint16_t box_x, uint16_t box_y)
{
   display.setTextColor(fgcolor);

   uint16_t box_w = display.width() - box_x - 10;
   uint16_t box_h = 120;
   printf("Partial update box x:%d y:%d width:%d height:%d\n", box_x, box_y, box_w, box_h);
   uint16_t cursor_y = box_y + 20;
   display.fillRect(box_x, box_y, box_w, box_h, bkcolor);
   display.setCursor(box_x, cursor_y + 40);
   display.println("PARTIAL");
   display.println("REFRESH");
   //display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void demo(uint16_t bkcolor, uint16_t fgcolor)
{
   display.fillScreen(bkcolor);
   display.setTextColor(fgcolor);
   display.setCursor(10, 40);
   display.setFont(&Ubuntu_M18pt8b);
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

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);

   //display.setRotation(2);
   //display.fillScreen(EPD_WHITE);
      // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   uint16_t foregroundColor = EPD_WHITE;
   // Make some rectangles showing the different colors or grays
   if (display.colors_supported>1) {
      printf("display.colors_supported:%d\n", display.colors_supported);
      foregroundColor = EPD_RED;
   }
   /* display.fillScreen(EPD_RED);
   display.update();
   return; // STOP  */
  
   uint16_t firstBlock = display.width()/4;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_WHITE);
   display.fillRect(rectW*2,1,rectW,firstBlock,foregroundColor); 
   display.fillRect(rectW*3,1,rectW-2,firstBlock,EPD_WHITE);

   display.fillRect(    1,firstBlock,rectW,firstBlock,EPD_BLACK);
   display.fillRect(rectW,firstBlock,rectW,firstBlock,foregroundColor);
   display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_BLACK); 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,foregroundColor);

   display.setCursor(display.width()/2-130,display.height()-104);
   display.setTextColor(EPD_WHITE);
   display.setFont(&Ubuntu_M18pt8b);
   display.println("BERLIN");
   display.setTextColor(EPD_BLACK);
   display.println("Gdeh042Z96 class for    Goodisplay 400x300");
   display.update();
   return;
}

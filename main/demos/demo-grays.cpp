#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Should match with your epaper module, size
//#include <gdep015OC1.h>  // 1.54 old version
//#include <gdeh0154d67.h>
//#include "wave12i48.h"
//#include <gdew042t2.h>  // Tested correctly 06.06.20
#include <gdew042t2Grays.h>
//#include <gdew0583t7.h>
//#include <gdew0213i5f.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>

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
Gdew042t2Grays display(io);
//Gdew042t2 display(io);
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M36pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void demo() {
   display.setTextColor(EPD_BLACK);
   display.setFont(&FreeMono12pt7b);
   
   display.setCursor(50,60);
   display.print("1 HELLO");
   display.setCursor(50,140);
   display.print("2 Twitter");

   display.drawRect(10, 2, display.width()-30, 80, EPD_BLACK);
   display.updateWindow(10, 2, display.width()-30, 79, true);
   delay(3000);

   
   display.drawRect(10, 80, display.width()-30, 100, EPD_BLACK);
   display.updateWindow(10, 80, display.width()-30, 100, true);
   delay(2000);

   return;
}
void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);

   // Test Epd class. true to enable debug
   display.init(0);
   display.setMonoMode(false);

   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   uint16_t foregroundColor = EPD_LIGHTGREY;
   //return;
   uint16_t firstBlock = display.width()/4;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_WHITE);
   display.fillRect(rectW*2,1,rectW,firstBlock,EPD_DARKGREY); 
   display.fillRect(rectW*3,1,rectW-2,firstBlock,EPD_WHITE);

   display.fillRect(    1,firstBlock,rectW,firstBlock,EPD_BLACK);
   display.fillRect(rectW,firstBlock,rectW,firstBlock,foregroundColor);
   display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_BLACK); 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,EPD_DARKGREY);
   display.update();
   delay(5000);

   display.fillScreen(EPD_WHITE);
   display.setCursor(display.width()/2-120,display.height()-120); 
   display.setCursor(20,40);
   display.setTextColor(EPD_BLACK);
   display.setFont(&FreeMono12pt7b);
   display.println("BLACK qwertzuiopasdABCD");
   display.setTextColor(EPD_LIGHTGREY);
   display.println("LG qwertzuiopasdABCD");
      display.setTextColor(EPD_DARKGREY);
   display.println("DG qwertzuiopasdABCD");

   display.fillRect(0,150, display.width(), 150, EPD_WHITE);
   display.fillCircle(60 , 200, 40, EPD_LIGHTGREY);
   display.fillCircle(160, 200, 40, EPD_DARKGREY);
   display.fillCircle(260, 200, 40, EPD_BLACK);
   display.drawCircle(360, 200, 40, EPD_BLACK);
   display.update();

   return; // Finish short demo here
   // Leave the epaper White ready for storage
   delay(5000);

   display.fillScreen(EPD_DARKGREY);
   display.update();
   delay(1000);

   display.fillScreen(EPD_LIGHTGREY);
   display.update();
   delay(1000);
   
   // Is getting inverted (But not on the font it seems, strange)
   display.fillScreen(EPD_WHITE);
   display.update();

   display.setCursor(10,display.height()-120);
   display.setTextColor(EPD_LIGHTGREY);
   display.println("Thanks for watching");
   display.update();
   delay(3000);
   display.fillRect(0,display.height()-140, display.width(), 100, EPD_WHITE);
   display.setCursor(10,display.height()-120);
   display.setTextColor(EPD_DARKGREY);
   display.println("FASANI CORPORATION");
   display.update();
   return;


   
}

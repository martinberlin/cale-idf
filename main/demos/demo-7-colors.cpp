#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "color/wave5i7Color.h"

// Single SPI EPD
EpdSpi io;
Wave5i7Color display(io);
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>

extern "C"
{
   void app_main();
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
  
   uint16_t firstBlock = display.width()/4;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_ORANGE);
   display.fillRect(rectW*2,1,rectW,firstBlock,foregroundColor); 
   display.fillRect(rectW*3,1,rectW-2,firstBlock,EPD_YELLOW);

   display.fillRect(    1,firstBlock,rectW,firstBlock,EPD_GREEN);
   display.fillRect(rectW,firstBlock,rectW,firstBlock,foregroundColor);
   display.fillRect(rectW*2,firstBlock,rectW,firstBlock,EPD_BLUE); 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,EPD_RED);

   display.setCursor(display.width()/2-130,display.height()-104);
   display.setTextColor(EPD_GREEN);
   display.setFont(&Ubuntu_M18pt8b);
   display.println("BERLIN");
   display.setTextColor(EPD_BLACK);
   display.println("wave5i7Color class for Waveshare 600x448 7 color epaper");
   display.update();

   return;
}

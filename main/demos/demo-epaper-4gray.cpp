#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Should match with your epaper module, size
//#include <gdew042t2Grays.h>  // Tested correctly 06.06.20
// New GOODISPLAY models
//#include "color/gdew0583z83.h"
#include "goodisplay/gdeq037T31.h"

EpdSpi io;
Gdeq037T31 display(io);

// Enable Power on Display
#define GPIO_DISPLAY_POWER_ON GPIO_NUM_4
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M100pt7b.h>
#include <Fonts/ubuntu/Ubuntu_M48pt8b.h>
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

void draw_content(uint8_t rotation){
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   // Make some rectangles showing the different colors or grays
   uint8_t rectW = display.width();
   if (display.getRotation() %2 == 0) {
     rectW = display.height();
   }

   rectW = display.width()/4; // For 11 is 37.
   display.fillScreen(EPD_WHITE);
   display.setFont(&Ubuntu_M24pt8b);
   uint16_t firstBlock = display.width()/8;
   display.fillRect(    1,1,rectW, firstBlock,EPD_BLACK);
   display.fillRect(rectW,1,rectW, firstBlock,EPD_DARKGREY);
   display.fillRect(rectW*2,1,rectW,firstBlock,EPD_LIGHTGREY);
   display.drawRect(rectW*3,1,rectW-2,firstBlock,EPD_LIGHTGREY); 

   display.setFont(&Ubuntu_M24pt8b);
   display.setTextColor(EPD_LIGHTGREY);
   display.setCursor(4, 180);
   display.println("HELLO");

   display.setTextColor(EPD_DARKGREY);
   display.println("TWITTER");

   display.setCursor(4, 34);
   display.setTextColor(EPD_WHITE);
   display.println("OK");

   display.setCursor(1, 90);
   display.setTextColor(EPD_BLACK);
   display.println("GOOD DISPLAY");
   display.update();
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Display PWR: 
   //gpio_set_direction(GPIO_DISPLAY_POWER_ON, GPIO_MODE_OUTPUT);
   //gpio_set_level(GPIO_DISPLAY_POWER_ON, 1);

   // Test Epd class
   display.init(false);
   display.setRotation(1);

   display.setMonoMode(true); // Monochrome demo
   display.fast_mode = 1;
   display.setTextColor(EPD_BLACK);
   for (int count=6; count>=0; count--) {
      display.fillScreen(EPD_WHITE);
      /* display.setCursor(20,10);
      display.printerf("%d", count); // Ubuntu_M100pt7b
      */
      display.draw_centered_text(&Ubuntu_M100pt7b, 0,-20, display.width(), display.height(), "%d", count);
      display.update();
      delay(100);
   }


   display.setMonoMode(false); // 4 grays

   display.setFont(&Ubuntu_M48pt8b);
   draw_content(1);
   delay(6500);

   
   display.setTextColor(EPD_WHITE);
   // fillScreen test:
   display.fillScreen(EPD_BLACK);
   display.draw_centered_text(&Ubuntu_M48pt8b, 0,0, display.width(), display.height(), "BLACK");
   display.update();
   delay(7000);
   display.fillScreen(EPD_DARKGREY);
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,0, display.width(), display.height(), "DARKGREY");
   display.update();
   delay(7000);
   display.fillScreen(EPD_LIGHTGREY);
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,0, display.width(), display.height(), "LIGHTGREY");
   display.update();
   delay(7000);
   display.fillScreen(EPD_WHITE);
   display.setTextColor(EPD_LIGHTGREY);
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,0, display.width(), display.height(), "WHITE");
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,150, display.width(), 100, "Good-Display.com");
   display.update();
   delay(10000);


   display.setMonoMode(true); // Monochrome demo
   display.setTextColor(EPD_WHITE);
   display.fast_mode = 1;
   for (int count=9; count>=0; count--) {
      display.fillScreen(EPD_BLACK);
      /* display.setCursor(20,10);
      display.printerf("%d", count); // Ubuntu_M100pt7b
      */
      display.draw_centered_text(&Ubuntu_M100pt7b, 0,0, display.width(), display.height(), "%d", count);
      display.update();
      delay(150);
   }

   display.fillScreen(EPD_WHITE);
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,100, display.width(), 50, "Thanks");
   display.draw_centered_text(&Ubuntu_M24pt8b, 0,150, display.width(), 50, "GOODISPLAY");
   display.update();
   delay(4000);
   display.fillScreen(EPD_WHITE);
   display.update();
   printf("display: We are done with the demo\n");
}

/**
 * plasticlogic.com Demo of 4 gray level epaper with partial refresh for ESP32 and ESP32S2
 * Proprietry process to manufacture truly flexible, organic thin-film transistors (OTFT),
 * with industrialization developed in the world’s first commercial, high volume, plastic electronics factory.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
// First plasticlogic EPD implementation
#include <plasticlogic011.h>

EpdSpi2Cs io;
PlasticLogic011 display(io);
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
extern "C"
{
   void app_main();
}

void demo(uint16_t bkcolor, uint16_t fgcolor)
{
   display.fillScreen(bkcolor);
   // Short test:
   for (int i = 1; i <= display.width(); i++)
   {
      display.drawPixel(i, 10, fgcolor);
   }
   display.setTextColor(fgcolor);
   display.setCursor(10, 40);
   display.setFont(&FreeMonoBold24pt7b);
   printf("display.width() %d\n\n", display.width());
   display.println("CalEPD display test\n");
   // Print all character from an Adafruit Font
   if (true)
   {
      for (int i = 40; i <= 126; i++)
      {
         display.write(i); // Needs to be >32 (first character definition)
      }
   }

   // Cope with different Epd resolutions just for the demo
   if (display.width() > 1000 && display.width() <= 1305)
   {
      display.fillCircle(650, 400, 180, fgcolor);
      display.fillCircle(900, 200, 40, fgcolor);
      display.fillCircle(1100, 200, 40, fgcolor);
      display.fillCircle(1100, 400, 40, fgcolor);
      display.fillCircle(1100, 700, 40, fgcolor);
   }
   else if (display.width() >= 800 && display.width() < 900)
   {
      display.setCursor(6, 626);
      display.fillRect(1, 600, display.width(), 34, fgcolor);
      display.fillCircle(100, 100, 60, fgcolor);
      display.fillCircle(200, 200, 50, fgcolor);
      display.fillCircle(300, 300, 40, fgcolor);
      display.fillCircle(500, 500, 200, fgcolor);
   }
   else if (display.width() > 300 && display.width() <= 400)
   {
      display.setCursor(6, 326);
      display.fillRect(1, 300, display.width(), 34, fgcolor);
      display.fillCircle(100, 100, 60, fgcolor);
      display.fillCircle(200, 200, 50, fgcolor);
      display.fillCircle(300, 300, 40, fgcolor);
   }

   display.setTextColor(bkcolor);
   display.setFont(&FreeMonoBold24pt7b);
   display.println("CalEPD");
   display.setTextColor(fgcolor);

   display.setFont(&FreeSerif12pt7b);
   display.println("AbcdeFghiJklm");

   display.setFont(&FreeSerifBoldItalic18pt7b);
   display.println("BERLIN");
   display.setFont(&FreeSansOblique24pt7b);
   display.println("is a very");
   display.println("nice city");
   return; // STOP test here
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
   display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void app_main(void)
{

   printf("CALE-IDF epaper research\n");
   /* Print chip information */
   esp_chip_info_t chip_info;
   esp_chip_info(&chip_info);
   printf("This is %s chip with %d CPU cores\n",
          CONFIG_IDF_TARGET,
          chip_info.cores);

   printf("Silicon revision %d, ", chip_info.revision);
   printf("Free heap: %d\n", esp_get_free_heap_size());

   // Test Epd class
   display.init(true);
   //display.clearScreen();
   display.fillCircle(50, 20, 10, EPD_BLACK);
   display.fillCircle(100, 30, 30, EPD_DARKGREY);
   //display.fillScreen(EPD_DARKGREY);
   
   display.setRotation(0); // Sets rotation in Adafruit GFX
   display.update();
   /* 

   // Back, Foreground
   //demo(EPD_WHITE, EPD_BLACK);
   display.setFont(&FreeSerifBoldItalic18pt7b);
   display.fillScreen(EPD_BLACK);
   display.setCursor(10,10);
   display.setTextColor(EPD_WHITE);
   display.print("HELLO");
   display.update();

   vTaskDelay(2000 / portTICK_PERIOD_MS);

   //demo(EPD_BLACK, EPD_WHITE);
   //display.update();
   // Partial update tests:
   // Note: Prints the background but not full black
   // As a side effect also affects the top and bottom parts minimally
   if (false)
   {
      demoPartialUpdate(EPD_BLACK, EPD_WHITE, 100, 50);
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      // Note:  This affects the white vertical all over the partial update so it's not usable. Do not use white background for now
      //demoPartialUpdate(EPD_WHITE, EPD_BLACK, 200, 100);
   } */
}

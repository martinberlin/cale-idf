#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
//#include <gdew042t2.h>
#include <gdew027w3.h>
//#include <gdeh0213b73.h>

EpdSpi io;
Gdew027w3 display(io);
//Gdew042t2 display(io);
//Gdeh0213b73 display(io);

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
extern "C" {
   void app_main();
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
   display.fillScreen(GxEPD_BLACK);
   display.setRotation(0);
   display.setFont(&FreeMonoBold18pt7b);
   
   display.setTextColor(GxEPD_WHITE); // Needs to invert color
   display.setCursor(5,30);
   display.println("CalEPD");
   display.println("MonoBold18pt7b");

   display.setFont(&FreeSerif12pt7b);
   display.println("");
   display.println("AbcdeFghiJklm");
   // This should be first test run!
   //display.fillScreen(GxEPD_BLACK);  // GxEPD_BLACK  GxEPD_WHITE
  display.update();
   return; 
 

// Print all character from an Adafruit Font
  if (false) {
   for (int i = 40; i <= 126; i++) {
      display.write(i); // Needs to be >32 (first character definition)
   }
   }
   // Test fonts
   display.setFont(&FreeMonoBold18pt7b);
   display.setCursor(100,50);
   display.println("Says hello BERLIN");

   // Test  shapes
   display.drawCircle(50, 150, 20, GxEPD_BLACK); // Adafruit works!
   display.drawCircle(50, 150, 22, GxEPD_BLACK);
   
   //display.drawRect(90, 50, 40, 20, GxEPD_BLACK);
   display.drawRect(90, 250, 38, 22, GxEPD_BLACK);

   display.drawRoundRect(134, 250, 20, 20, 5, GxEPD_BLACK);

   //display.drawTriangle(174, 50, 184, 60, 200, 50, GxEPD_BLACK);

   for (int i = 0; i < 200; i++) {
     display.drawPixel(i,279,GxEPD_BLACK);
     display.drawPixel(i,280,GxEPD_BLACK);
   }

   display.update();
   vTaskDelay(100);
   
   //display.fillScreen(GxEPD_WHITE);
   //display.update();
   
   // Partial updates are not working in this display
   //display.updateWindow(10,20,80,100);
}

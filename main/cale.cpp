#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include <gdew042t2.h>
//#include <gdeh0213b73.h>
//#include <gdeh0213b73.h>

EpdSpi io;
Gdew042t2 display(io);
//Gdeh0213b73 display(io);
//Gdew0213i5f display(io);

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
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
   display.setRotation(0);
   display.setFont(&FreeMono9pt7b);
   display.setTextColor(GxEPD_BLACK);
   display.setCursor(5,20);
   display.println("HELLO");
   // This should be first test run!
   /* display.fillScreen(GxEPD_BLACK);  // GxEPD_BLACK  GxEPD_WHITE
   display.update();
   return; */
 

// Print all character from an Adafruit Font
  if (false) {
   for (int i = 40; i <= 126; i++) {
      display.write(i); // Needs to be >32 (first character definition)
   }
   }
   // Test fonts
   display.println("CALE ESP-IDF");  // Todo: Add print and println
   display.setFont(&FreeMono18pt7b);
   display.setCursor(10,40);
   display.println("Says hello");
   display.setCursor(10,100);
   display.println("BERLIN");
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

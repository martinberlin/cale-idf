#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <epd.h>
// FONT used for title / message body
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/FreeMono9pt7b.h>

extern "C" {
   void app_main();
}

Epd display;

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
   //display.setFont(&FreeMono9pt7b);
   display.setTextColor(GxEPD_BLACK);
   display.setCursor(20,20);
   
   // Try to print characters
   for (int i = 0; i < 80; i++) {
      display.write(i); // Does not work
   }

   const char c[4] = "abc";
   //display.println(c);  // Does not work


   display.setRotation(1);
   display.fillScreen(GxEPD_WHITE); 

   display.drawCircle(50, 50, 20, GxEPD_BLACK); // Adafruit works!
   display.drawCircle(50, 50, 22, GxEPD_BLACK);
   
   for (int i = 0; i < 200; i++) {
     display.drawPixel(i,15,GxEPD_BLACK);
     //display.drawPixel(i,90,GxEPD_WHITE);
   }
   display.update();
}

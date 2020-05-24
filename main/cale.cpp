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
   display.setFont(&FreeMono9pt7b);
   display.setTextColor(GxEPD_WHITE);
   
   //display.print("HELLO CALE-IDF");

   display.setRotation(1);
   display.fillScreen(GxEPD_BLACK); 

   display.drawCircle(30, 30, 10,GxEPD_WHITE); // Adafruit works

   
   for (int i = 0; i < 200; i++) {
     display.drawPixel(i,15,GxEPD_WHITE);
     //display.drawPixel(i,90,GxEPD_WHITE);
   }
   display.update();
}

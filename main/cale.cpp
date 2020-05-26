#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <epd.h>
// FONT used for title / message body
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
extern "C" {
   void app_main();
}
EspSpi io;
Epd display(io);

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


   display.setRotation(1);
   display.fillScreen(GxEPD_WHITE); 

   display.setFont(&FreeMono9pt7b);
   display.setTextColor(GxEPD_BLACK);
   display.setCursor(20,20);

// Print all character from an Adafruit Font
  if (false) {
   for (int i = 40; i <= 126; i++) {
      display.write(i); // Needs to be >32 (first character definition)
   }
   }
   // Test fonts
   display.println("HELLO CALE-IDF");  // Todo: Add print and println
   display.setFont(&FreeMono18pt7b);
   display.setCursor(10,40);
   display.println("Adafruit GFX");
   display.setCursor(10,100);
   display.println("F O N T S");
   // Test  shapes
   display.drawCircle(50, 50, 20, GxEPD_BLACK); // Adafruit works!
   display.drawCircle(50, 50, 22, GxEPD_BLACK);
   
   display.drawRect(90, 50, 40, 20, GxEPD_BLACK);
   display.drawRect(90, 50, 38, 22, GxEPD_BLACK);

   display.drawRoundRect(134, 50, 20, 20, 5, GxEPD_BLACK);

   display.drawTriangle(174, 50, 184, 60, 194, 20, GxEPD_BLACK);

   for (int i = 0; i < 200; i++) {
     display.drawPixel(i,79,GxEPD_BLACK);
   }
   display.update();
}

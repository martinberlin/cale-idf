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
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
extern "C" {
   void app_main();
}

void demo(uint16_t bkcolor,uint16_t fgcolor){

   display.fillScreen(bkcolor);
   display.setTextColor(fgcolor);
   display.setCursor(1,30);
   display.setFont(&FreeMono9pt7b);
   display.println("MonoBold18pt7b");
   // Short test:
   /* for (int i = 0; i < 10; i++) {
     display.drawPixel(i,0,fgcolor);
   } 
   return; */
   display.setCursor(6,62);
   display.fillRect(1, 40, display.width(), 30, fgcolor);
   display.setTextColor(bkcolor);
   display.setFont(&FreeMonoBold18pt7b);
   display.println("CalEPD");
   display.setTextColor(fgcolor);

   display.setFont(&FreeMono9pt7b);
   display.println("Serif12pt7b");

   display.setFont(&FreeSerif12pt7b);
   display.println("AbcdeFghiJklm");

   display.setFont(&FreeSerifBoldItalic18pt7b);
   display.println("BERLIN");

   // Test  shapes
   display.drawCircle(50, 190, 20, fgcolor); 
   display.drawCircle(50, 190, 22, fgcolor);

   display.drawRect(90, 200, 38, 22, fgcolor);

   display.drawTriangle(174, 150, 100, 60, 200, 50, fgcolor);

   for (int i = 0; i < 200; i++) {
     display.drawPixel(i,279,fgcolor);
     display.drawPixel(i,280,fgcolor);
   } 
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

   

// Print all character from an Adafruit Font
  if (false) {
   for (int i = 40; i <= 126; i++) {
      display.write(i); // Needs to be >32 (first character definition)
   }
   }
   
   demo(GxEPD_BLACK,GxEPD_WHITE);
   display.update();

   vTaskDelay(5000 / portTICK_PERIOD_MS);

   demo(GxEPD_WHITE,GxEPD_BLACK);
   display.update();
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include "wave12i48.h"
//#include <gdew042t2.h>  // Tested correctly 06.06.20
//#include <gdew0583t7.h>
//#include <gdew075T7.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>

Epd4Spi io;
Wave12I48 display(io);

//Gdew0583T7 display(io);
//Gdew075T7 display(io);
//Gdew027w3 display(io);
//Gdeh0213b73 display(io); // Does not work correctly yet - moved to /fix

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
extern "C" {
   void app_main();
}

void demo(uint16_t bkcolor,uint16_t fgcolor){
   display.fillScreen(bkcolor);
   // Short test:
   for (int i = 1; i <= display.width(); i++) {
     display.drawPixel(i,10,fgcolor);
   }
   display.setTextColor(fgcolor);
   display.setCursor(10,40);
   display.setFont(&FreeMonoBold24pt7b);
   display.println("CalEPD testing GFX engine");

   display.fillCircle(650, 400, 180, fgcolor);
   display.fillCircle(900, 200, 40, fgcolor);
   display.fillCircle(1100, 200, 40, fgcolor);
   display.fillCircle(1100, 400, 40, fgcolor);
   display.fillCircle(1100, 700, 40, fgcolor);
    
   display.setCursor(10,80);
   display.setFont(&FreeMono9pt7b);
   display.println("FreeMonoBold24pt7b test");
   display.setFont(&FreeMonoBold24pt7b);
   display.println("We are unable to offer you data");
   display.println("about the IC controller of the e-paper,");
   display.println("according to our engineer.");
   display.setCursor(6,626);
   display.fillRect(1, 600, display.width(), 34, fgcolor);
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
}

void demoPartialUpdate(uint16_t bkcolor,uint16_t fgcolor,uint16_t box_x,uint16_t box_y)
{
  display.setTextColor(fgcolor);
  
  uint16_t box_w = display.width()-box_x-10;
  uint16_t box_h = 120;
  printf("Partial update box x:%d y:%d width:%d height:%d\n",box_x,box_y,box_w,box_h);
  uint16_t cursor_y = box_y + 20;
  display.fillRect(box_x, box_y, box_w, box_h, bkcolor);
  display.setCursor(box_x, cursor_y+40);
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
   
   display.setRotation(0);

// Print all character from an Adafruit Font
  if (false) {
   for (int i = 40; i <= 126; i++) {
      display.write(i); // Needs to be >32 (first character definition)
   }
   }
   // Back, Foreground
   //demo(EPD_BLACK,EPD_WHITE);
   demo(EPD_WHITE,EPD_BLACK);
   display.update();
   

   /* demo(EPD_WHITE,EPD_BLACK);
   display.update();
   
   vTaskDelay(2000 / portTICK_PERIOD_MS);
   // Partial update tests:
  // Note: Prints the background but not full black
  // As a side effect also affects the top and bottom parts minimally
  if (true) {
   demoPartialUpdate(EPD_BLACK, EPD_WHITE, 100, 50);
   vTaskDelay(3000 / portTICK_PERIOD_MS);
   // Note:  This affects the white vertical all over the partial update so it's not usable. Do not use white background for now
   //demoPartialUpdate(EPD_WHITE, EPD_BLACK, 200, 100);
   } */
}

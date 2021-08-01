#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "image_array/image4gray.h"
#include <stdlib.h>     /* srand, rand */
// Should match with your epaper module, size
//#include <gdep015OC1.h>  // 1.54 old version
//#include <gdeh0154d67.h>
//#include "wave12i48.h"
//#include <gdew042t2.h>  // Tested correctly 06.06.20
//#include <gdew0583t7.h>
//#include <gdew075T7.h>
#include <gdew075T7Grays.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>

// Color
//#include <gdew0583z21.h>
//#include <gdeh042Z96.h>
//#include <gdeh042Z21.h>
// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
/* Epd4Spi io;
Wave12I48 display(io); */

// Single SPI EPD
EpdSpi io;
Gdew075T7Grays display(io);
//Gdep015OC1 display(io);
//Gdeh0213b73 display(io); // Does not work correctly yet - moved to /fix

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

uint16_t randomNumber(uint16_t max) {
  srand(esp_timer_get_time());
  return rand()%max;
}

void demoPartialUpdate(uint16_t bkcolor, uint16_t fgcolor, uint16_t box_x, uint16_t box_y)
{
   uint16_t box_w = display.width() - box_x - 10;
   uint16_t box_h = 60;
   printf("Partial update box x:%d y:%d width:%d height:%d\n", box_x, box_y, box_w, box_h);
   uint16_t cursor_y = box_y + 20;
   display.fillRect(box_x, box_y, box_w, box_h, bkcolor);
   display.setCursor(box_x, cursor_y + 20);
   display.setFont(&Ubuntu_M18pt8b);
   display.setTextColor(fgcolor);
   display.println("PARTIAL UPDATE");
   //display.update(); // Full update works good
   // Partial does not (Black is not full black)
   //display.updateToWindow(box_x, box_y, 0,0,box_w, box_h,true);
   display.updateWindow(box_x, box_y, box_w, box_h,true);
}

void demo(uint16_t bkcolor, uint16_t fgcolor)
{
   display.fillScreen(bkcolor);
   display.setTextColor(fgcolor);
   display.setCursor(10, 40);
   display.setFont(&Ubuntu_M18pt8b);
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

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);
         // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   uint16_t foregroundColor = EPD_WHITE;
   /* printf("--> Test clear Screen\n");
   display.fillScreen(0x00);
   display.update(); */
   
   display.fillCircle(100, 100, 50, 230); // Light gray

   display.fillCircle(200, 100, 50, EPD_LIGHTGREY);

   display.fillCircle(300, 100, 50, 140);

   display.fillCircle(400, 100, 50, 100);

   display.fillCircle(500, 100, 50, 50);

   display.fillCircle(600, 100, 50, 10);   // BLACK
   
   display.update();
   // Image is not working since it's too big, using EXT_RAM_ATTR is for some reason delivering only 0's
   //printf("--> Test image from array size:%d\n", sizeof(imageGrays1));
   //display.fillRawBufferImage(imageGrays1, sizeof(imageGrays1));
   
   delay(1000); 


}

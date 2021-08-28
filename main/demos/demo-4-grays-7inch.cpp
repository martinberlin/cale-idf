#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>     /* srand, rand */
#include <gdew075T7Grays.h>

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
   
   display.fillCircle(100, 100, 50, EPD_BLACK);
   display.fillCircle(200, 100, 50, EPD_DGRAY);
   display.fillCircle(300, 100, 50, EPD_LGRAY);
   display.fillCircle(400, 100, 50, EPD_WHITE);

   display.fillRect(20, 150, 500, 100, EPD_BLACK);
   display.fillCircle(100, 200, 50, EPD_BLACK);
   display.fillCircle(200, 200, 50, EPD_DGRAY);
   display.fillCircle(300, 200, 50, EPD_LGRAY);
   display.fillCircle(400, 200, 50, EPD_WHITE);
   
   display.update();
}

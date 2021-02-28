#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Only for parallel epaper displays driven by I2S DataBus (No SPI)
// NOTE: This needs Epdiy component https://github.com/vroland/epdiy
// Run idf.py menuconfig-> Component Config -> E-Paper driver and select:
// Display type: LILIGO 4.7 ED047TC1
// Board: LILIGO T5-4.7 Epaper
// In the same section Component Config -> ESP32 Specifics -> Enable PSRAM
#include "parallel/ED047TC1.h"
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h>
Ed047TC1 display;

bool cleanScreenAtStart = true;
extern "C"
{
   void app_main();
}

void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

uint16_t randomGrayColor() {
  srand(esp_timer_get_time());
  uint8_t random = rand()%5;
  uint16_t color = 0x33;
  switch (random)
  {
  case 0:
     color = EPD_BLACK;
     break;
  case 1:
     color = EPD_SDGRAY;
     break;
  case 2:
     color = EPD_DGRAY;
     break;
  case 3:
     color = EPD_GRAY;
     break;
  case 4:
     color = EPD_LGRAY;
     break;
  case 5:
     color = EPD_SLGRAY;
     break;
  }
  return color;
}

uint16_t randomNumber(uint16_t max) {
  srand(esp_timer_get_time());
  return rand()%max;
}

void printHey(){
   /* display.setCursor(20,30);
   display.setTextColor(EPD_LGRAY);
   display.setFont(&Ubuntu_M24pt8b);
   display.print("HEY");
   display.fillCircle(50,45,15,EPD_LGRAY);
   */
   display.fillCircle(50,50,50, EPD_LGRAY);
}

void app_main(void)
{

   display.init(true);
   //display.setRotation(2); // Rotate screen 180°. Default is 0

   // Clear all screen to white
   if (cleanScreenAtStart) {
     display.clearScreen();   
     delay(1000);
     //Stop here if you just want to clean epaper screen:
     //return;
   }
   display.setCursor(20,250);
   display.setTextColor(EPD_DGRAY);
   display.setFont(&Ubuntu_M24pt8b);
   display.print("Demo partial update");
   // Taken from C example: https://www2.cs.sfu.ca/CourseCentral/166/tjd/bouncing_ball.html
   // The bouncing ball test
   uint16_t x = 100;
   uint16_t y = 100;
   uint16_t lastX = x;
   uint16_t lastY = x;
   uint16_t count = 0;
   uint16_t dx = 14;
   uint16_t dy = 14;
   uint16_t radius = 30;
   uint16_t totalFrames = 500;

   // Don't do this forever! It's a lot of phisical work for the epaper...
   while (count<totalFrames) { 
      // Delete last position
      display.fillCircle(lastX,lastY,radius*2, EPD_WHITE);
      display.updateWindow(lastX-radius,lastY-radius,radius*2,radius*2,WHITE_ON_BLACK);

      x += dx;
      y += dy;

      // hit the left edge?
      if (x - radius <= 0) {
         dx = -dx;
         x = radius;
      }

      // hit the right edge?
      if (x + radius >= display.width()) {
         dx = -dx;
         x = display.width() - radius*2;
      }

      // hit the top edge?
      if (y - radius <= 0) {
         dy = -dy;
         //radius += 10;
         y = radius;
      }

      // hit the bottom edge?
      if (y + radius >= display.height()) {
         dy = -dy;
         y = display.height() - radius;
      }
      
      display.fillCircle(x,y,radius, EPD_BLACK);
      display.updateWindow(x-radius,y-radius,radius*2,radius*2);
      lastX =x;
      lastY =y;
      ++count;
      if (count%100==0) {
         printf("%d frames rendered. Still %d left\n", count, totalFrames-count);
      }
   }
 
 display.clearScreen();   
 delay(1000);
}

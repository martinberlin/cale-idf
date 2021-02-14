#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <gdeh0154d67.h>
// Single SPI EPD
EpdSpi io;
Gdeh0154d67 display(io);

bool cleanScreenAtStart = true;
extern "C"
{
   void app_main();
}

void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void app_main(void)
{

   display.init(false);
   //display.setRotation(2); // Rotate screen 180°. Default is 0

   // Clear all screen to white
   if (cleanScreenAtStart) {
     display.update();
   }
   
   // Taken from C example: https://www2.cs.sfu.ca/CourseCentral/166/tjd/bouncing_ball.html
   // The bouncing ball test
   uint16_t x = 10;
   uint16_t y = 10;
   uint16_t lastX = x;
   uint16_t lastY = x;
   uint16_t count = 0;
   uint16_t dx = 14;
   uint16_t dy = 14;
   uint16_t radius = 14;
   uint16_t totalFrames = 500;

   // Don't do this forever! It's a lot of phisical work for the epaper...
   while (count<totalFrames) { 
      // Delete last position
      display.fillCircle(lastX,lastY,radius*2, EPD_WHITE);
      display.updateWindow(lastX-radius,lastY-radius,radius*2,radius*2); 

      x += dx;
      y += dy;

      // hit the left edge?
      if (x - radius <= 0) {
         dx = -dx;
         x = radius;
      }

      // hit the right edge?
      if (x + radius >= display.width()-1) {
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
      if (y + radius >= display.height()-1) {
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
 
}

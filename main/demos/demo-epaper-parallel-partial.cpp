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
   }

   // Partial update
   //display.fillCircle(x,y,r,EPD_BLACK);
   uint16_t radius = 50;
   uint16_t increment = 30;
   display.setCursor(20,250);
   display.setTextColor(EPD_DGRAY);
   display.setFont(&Ubuntu_M24pt8b);
   display.print("Demo partial update");
   /* display.update();
   return; */
   display.updateWindow(10,200,500,100);

   for (int16_t x = 0; x < display.width()-(radius*2); x=x+increment) { 
      if (x) {
       display.fillCircle(x+radius-increment,150,radius, EPD_WHITE);
       display.updateWindow(x-increment,100,radius*2,radius*2, WHITE_ON_BLACK);
      }
      display.fillCircle(x+radius,150,radius, EPD_DGRAY);
      display.updateWindow(x,100,radius*2,100);
   }
   for (int16_t x = display.width(); x > radius*2; x=x-increment) { 
      if (x) {
       display.fillCircle(x+radius+increment,150,radius, EPD_WHITE);
       display.updateWindow(x+increment,100,radius*2,radius*2, WHITE_ON_BLACK);
      }
      display.fillCircle(x+radius,150,radius, EPD_DGRAY);
      display.updateWindow(x,100,radius*2,100);
   }
}

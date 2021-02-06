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
   display.setCursor(20,30);
   display.setTextColor(EPD_DGRAY);
   display.setFont(&Ubuntu_M24pt8b);
   display.print("HEY");
}

void app_main(void)
{

   display.init(true);
   //display.setRotation(1); // Rotate screen °90

   // Clear all screen to white
   if (cleanScreenAtStart) {
     display.clearScreen();   
     delay(1000);
   }
   uint16_t x = 100;
   uint16_t y = 100;
   uint16_t r = 60;
   //uint16_t w = 100;
   //uint16_t h = 200;
   //

   // Partial update
   //display.fillCircle(x,y,r,EPD_BLACK);
   
   printHey();
   display.updateWindow(0,0,200,200);
   
   
   delay(3000);
   display.clearScreen(); 
   display.fillScreen(EPD_WHITE);
   printHey();
   display.update();
}

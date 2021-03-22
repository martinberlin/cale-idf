#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Only for parallel epaper displays driven by I2S DataBus (No SPI)
// NOTE: This needs Epdiy component https://github.com/vroland/epdiy
// Run idf.py menuconfig-> Component Config -> E-Paper driver and select:
// Display type: LILIGO 4.7 ED047TC1
// Board: LILIGO T5-4.7 Epaper
// In the same section Component Config -> ESP32 Specifics -> Enable PSRAM
//#include "parallel/ED047TC1.h"
//Ed047TC1 display;
#include "parallel/ED060SC4.h"
Ed060SC4 display;

// Include a font
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M36pt7b.h>

// Play settings
uint16_t msDelayBetweenSlides = 3000; // 3 seconds
bool cleanScreenAtStart = false;

extern "C"
{
   void app_main();
}

void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void nextSlide() {
   delay(msDelayBetweenSlides);
   display.clearScreen();
   display.fillScreen(EPD_WHITE);
}

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


void app_main(void)
{
   display.init(true);
   //display.setRotation(1); // Rotate screen °90

   // Clear all screen to white
   if (cleanScreenAtStart) {
     display.clearScreen();   
     delay(1000);
   }

   // Draw some Random 14px height lines
   uint8_t lineHeight = display.height()/30; // 18 for 540
   for (uint16_t repeat = 1; repeat <= display.height(); repeat = repeat+lineHeight)
   {
      display.fillRect(0, repeat, display.width(), lineHeight, randomGrayColor());
   }
   display.update();
   nextSlide();

   // Draw some gray circles
   for (uint16_t repeat = 1; repeat <= 150; repeat++)
   {
      uint8_t radius = randomNumber(40);
      display.fillCircle(randomNumber(display.width()+radius), randomNumber(display.height()+radius), radius, randomGrayColor());
   }
   display.update();
   nextSlide();

   // Draw some gray triangles
   for (uint16_t repeat = 1; repeat <= 50; repeat++)
   {
      uint8_t radius = randomNumber(40);
      uint16_t x = repeat*18;
      uint16_t y = randomNumber(display.height()-radius);
      uint16_t z = x+radius;
      display.fillTriangle(x, y, z, x-radius, y-randomNumber(40),z-randomNumber(40), randomGrayColor());
   }
   display.update();
   nextSlide();

   // Draw some rectangles pixel by pixel
   for (int x = 0; x < 200; x++) {
      for (int y = 0; y < display.height(); y++) {
         // Of course you can do this easier with GFX fillRect like in test above.
         display.drawPixel(x, y, 0);
         display.drawPixel(x+200, y, 80);
         display.drawPixel(x+400, y, 160);
         display.drawPixel(x+600, y, 200);
         display.drawPixel(x+760, y, 230);
      }
   }

   // Draw some text with the Ubuntu font
   display.setTextColor(255);
   display.setCursor(30,200);
   display.setFont(&Ubuntu_M36pt7b);
   display.println("Hello world");
   display.setCursor(30,250);
   display.setFont(&Ubuntu_M24pt8b);
   display.println("This is a TTGO T5 4.7 inch epaper");
   display.update();

   delay(10000);
   display.clearScreen();
   display.fillScreen(EPD_WHITE);
   display.update();
}

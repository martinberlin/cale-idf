#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "color/wave5i7Color.h"
#include <stdlib.h>     /* srand, rand */

// Single SPI EPD
EpdSpi io;
Wave5i7Color display(io);
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>
uint16_t msDelayBetweenSlides = 14000; // 14 seconds

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

uint16_t randomNumber(uint16_t max) {
  srand(esp_timer_get_time());
  return rand()%max;
}

uint16_t randomColor() {
  srand(esp_timer_get_time());
  uint8_t random = rand()%5;
  uint16_t color = 0x33;
  switch (random)
  {
  case 0:
     color = EPD_GREEN;
     break;
  
  case 1:
     color = EPD_RED;
     break;
  case 2:
     color = EPD_ORANGE;
     break;
  case 3:
     color = EPD_YELLOW;
     break;
  case 4:
     color = EPD_BLUE;
     break;
  case 5:
     color = EPD_BLACK;
     break;
  }
  return color;
}

void nextSlide() {
   delay(msDelayBetweenSlides);
   display.fillScreen(EPD_WHITE);
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);
   // Update to: false if SPI optimization fails to work as expected
   display.spi_optimized = true;

   // Remove ghosts from last refresh (Didn't find any other workaround)
   // check: https://github.com/martinberlin/cale-idf/wiki/Model-color-wave5i7color.h#known-issues
   display.fillScreen(EPD_WHITE);
   display.update();
   nextSlide();

   //display.setRotation(2);
   // Draw some squares
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   uint16_t foregroundColor = EPD_WHITE;
  
   uint16_t firstBlock = display.width()/4;
   display.fillRect(    1,1,rectW, firstBlock,foregroundColor);
   display.fillRect(rectW,1,rectW, firstBlock,randomColor());
   display.fillRect(rectW*2,1,rectW,firstBlock,randomColor()); 
   display.fillRect(rectW*3,1,rectW-2,firstBlock,randomColor());

   display.fillRect(    1,firstBlock,rectW,firstBlock,randomColor());
   display.fillRect(rectW,firstBlock,rectW,firstBlock,randomColor());
   display.fillRect(rectW*2,firstBlock,rectW,firstBlock,randomColor()); 
   display.fillRect(rectW*3,firstBlock,rectW-2,firstBlock,randomColor());

   display.setCursor(display.width()/2-130,display.height()-132);
   display.setTextColor(randomColor());
   display.setFont(&Ubuntu_M18pt8b);
   display.println("BERLIN");
   display.setTextColor(EPD_BLACK);
   display.println("wave5i7Color class for Waveshare\n600x448 7 color epaper\nNext slide in 2 seconds >");
   display.update();
   nextSlide();

   // Draw some Random color small 2x2 squares in center of the screen
   for (uint16_t repeat = 1; repeat <= 10000; repeat++)
   {
      display.fillRect(randomNumber(display.width()), randomNumber(display.height()), 2, 2, randomColor());
   }
   display.update();
   nextSlide();

   // Draw some Random 14px height lines
   uint8_t lineHeight = display.height()/32; // 14 for 448
   for (uint16_t repeat = 1; repeat <= display.height(); repeat = repeat+lineHeight)
   {
      display.fillRect(0, repeat, display.width(), lineHeight, randomColor());
   }
   display.update();
   nextSlide();

   // Draw some color circles
   for (uint16_t repeat = 1; repeat <= 100; repeat++)
   {
      uint8_t radius = randomNumber(30);
      display.fillCircle(randomNumber(display.width()+radius), randomNumber(display.height()+radius), radius, randomColor());
   }
   display.update();
   
   // Do your own example using all GFX functions you want (Check Adafruit GFX documentation)
   return;
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Should match with your epaper module, size

// New small GOODISPLAY models
#include "small/gdew0102I4FC.h"


// Single SPI EPD
EpdSpi io;
Gdew0102I4FC display(io);


// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }


void demo(uint16_t bkcolor, uint16_t fgcolor)
{
   display.fillScreen(bkcolor);
   display.setTextColor(fgcolor);
   display.setCursor(10, 40);
   display.setFont(&Ubuntu_M12pt8b);
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

void draw_content(uint8_t rotation){
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   
   display.setTextColor(EPD_BLACK);
   display.setFont(&Ubuntu_M12pt8b);
   
   uint8_t rectW = display.width();
   uint8_t rectH = display.height();
   if (display.getRotation() %2 == 0) {
     rectW = display.height();
     rectH = display.width();
   }
   display.fillRect(1,1,11,11,EPD_BLACK);
   display.fillRect(1,rectW-10,11,11,EPD_BLACK);
   display.fillRect(rectH-10,0,10,10,EPD_BLACK);
   display.fillRect(rectH-10,rectW-10,10,10,EPD_BLACK);
   display.setCursor(10, rectH/2-12);
   display.print("Hello!");
   display.update();
   delay(2000);
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // 5V is not enought for Front light if there are Leds in serie
   //gpio_set_level(GPIO_ENABLE_5V, 1);

   // Test Epd class
   display.init(false);
   display.setFont(&Ubuntu_M12pt8b);
   display.fillCircle(30,display.height()/2,30,EPD_BLACK);
   display.update();
   delay(1000);
   display.fillScreen(EPD_WHITE);
   //return; // Just clean display and draw a circle

   display.setRotation(0);
   draw_content(display.getRotation());
   display.fillScreen(EPD_WHITE);
   delay(1000);
   display.setRotation(1);
   draw_content(display.getRotation());
   delay(1000);
   display.fillScreen(EPD_WHITE);
   display.update();
   printf("display: We are done with the demo");
}

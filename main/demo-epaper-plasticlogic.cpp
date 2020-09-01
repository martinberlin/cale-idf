/**
 * plasticlogic.com Demo of 4 gray level epaper with partial refresh for ESP32 and ESP32S2
 * Proprietry process to manufacture truly flexible, organic thin-film transistors (OTFT),
 * with industrialization developed in the world’s first commercial, high volume, plastic electronics factory.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module and size
// One or many classes can be included at the same time
#include <plasticlogic011.h>
#include <plasticlogic014.h>
#include <plasticlogic021.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic011 display(io);
//PlasticLogic014 display(io);
//PlasticLogic021 display(io);
bool playShortDemo = true;


extern "C"
{
   void app_main();
}

// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M36pt7b.h>

void print_plastic_logic(std::string text, uint16_t color) {
   display.setFont(&Ubuntu_M16pt8b);
   display.setCursor(2,20);
   display.setTextColor(EPD_DGRAY);
   display.print("Plastic");
   display.setCursor(2,44);
   display.print(text);
}

void print_title(std::string title, uint16_t color = EPD_BLACK, int16_t x = 2){
   display.setFont(&Ubuntu_M12pt8b);
   display.setCursor(x,15);
   display.setTextColor(color);
   display.print(title);
}

void app_main(void)
{
   printf("CalEPD version: %s for Plasticlogic.com\n", CALEPD_VERSION);
   
   /** Color constants that the epaper supports:
    EPD_BLACK 0x00
    EPD_DGRAY 0x01
    EPD_LGRAY 0x02
    EPD_WHITE 0x03
    */
   
   // Initialize display class
   display.init();         // Add init(true) for debug

   // TODO: Rotation is not working as it should:
   
   display.clearScreen();
   // GFX Rotation not working as in others when > 1.1" 
   // Suspected reason: scrambleBuffer/getPixel should be also rotation aware
   //display.setRotation(0);    // Does not work in 2.1 due to scrambleBuffer()

   //display.setEpdRotation(2); // 2: Does not turn it portrait, just upside down (Same with Paperino PL_microEPD)
   print_plastic_logic("logic.com", EPD_DGRAY);  // Prints plasticlogic.com

   display.fillRect(1,54, display.width(), 4, EPD_BLACK);
   display.update();
   
   vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait N seconds
   display.clearScreen();
   display.update();

   // Print temperature
   uint16_t delayPartial = 500;
   display.setFont(&Ubuntu_M24pt8b);

   printf("display.width: %d pix\n", display.width());
   switch (display.width())
   {
   case 148:
      display.setCursor(30,40);
      break;
   case 240:
      display.setFont(&Ubuntu_M36pt7b);
      delayPartial = 1000;
      printf("setting cursor to x:%d y:%d\n", 40,190);
      display.setCursor(40,90);
      break;
   default:
      display.setCursor(50,50);
      break;
   }
   printf("celsius: %d degrees", display.readTemperature());
   display.print(display.readTemperatureString('c')); // use 'f' for fahrenheit
   display.update();

   vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait N seconds
   display.clearScreen();  // Do a clearScreen between frames otherwise last one will remain and only new parts overwritten
   display.update();
   
   
   // Sizes are calculated dividing the screen in 4 equal parts it may not be perfect for all models
   uint8_t rectW = display.width()/4; // For 11 is 37.

   // Make some rectangles showing the different shades of gray
   // fillRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color)
   display.fillRect(    1,1,rectW,display.height(),EPD_BLACK);
   display.fillRect(rectW,1,rectW,display.height(),EPD_DGRAY);   // Dark gray
   display.fillRect(rectW*2,1,rectW,display.height(),EPD_LGRAY); // Light gray
   display.drawRect(rectW*3,1,rectW,display.height(),EPD_BLACK); // Last white rectangle
   display.update();

   vTaskDelay(2000 / portTICK_PERIOD_MS);
   display.clearScreen();
   print_title("3 grays",20);

   // Draw 3 circles showing the levels of gray
   // fillRect(int16_t x, int16_t y, int16_t radius, int16_t color)
   display.fillCircle(rectW, display.height()/2, 15, EPD_BLACK);
   display.fillCircle(rectW*2, display.height()/2, 15, EPD_DGRAY);
   display.fillCircle(rectW*3, display.height()/2, 15, EPD_LGRAY);
   display.update();
   vTaskDelay(2000 / portTICK_PERIOD_MS);
   display.clearScreen();

   if (playShortDemo) {
   display.update();
   return; // STOP
   }
   // MONO does not support grays:
   uint8_t delayMono = 100;
   print_title("mono update", EPD_BLACK, 1);
   display.fillCircle(rectW, display.height()/2, 20, EPD_BLACK);
   display.update(); // full update
   vTaskDelay(delayMono / portTICK_PERIOD_MS);
   display.fillCircle(rectW*2, display.height()/2, 20, EPD_BLACK);
   display.update(EPD_UPD_MONO);
   vTaskDelay(delayMono / portTICK_PERIOD_MS);
   display.fillCircle(rectW*3, display.height()/2, 20, EPD_BLACK);
   display.update(EPD_UPD_MONO);
   vTaskDelay(delayMono / portTICK_PERIOD_MS);
   display.clearScreen();

   print_title("partial update", EPD_DGRAY, 1);
   display.fillCircle(rectW, display.height()/2, 15, EPD_LGRAY);
   display.update(); // full update
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.fillCircle(rectW*2, display.height()/2, 15, EPD_DGRAY);
   display.update(EPD_UPD_PART);
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.fillCircle(rectW*3, display.height()/2, 15, EPD_BLACK);
   display.update(EPD_UPD_PART);
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.clearScreen();

   display.fillCircle(rectW*3, display.height()/2, 15, EPD_DGRAY);
   display.update();
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.fillCircle(rectW*2, display.height()/2, 15, EPD_LGRAY);
   display.update(EPD_UPD_PART);
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.drawCircle(rectW, display.height()/2, 15, EPD_BLACK);
   display.update(EPD_UPD_PART);
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   print_title("end", EPD_DGRAY, 54);
   display.update(EPD_UPD_PART);
   vTaskDelay(delayPartial / portTICK_PERIOD_MS);
   display.clearScreen();

}

/**
 * plasticlogic.com Demo of 4 gray level epaper with partial refresh for ESP32 and ESP32S2
 * Proprietry process to manufacture truly flexible, organic thin-film transistors (OTFT),
 * with industrialization developed in the world’s first commercial, high volume, plastic electronics factory.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
// First plasticlogic EPD implementation
#include <plasticlogic011.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic011 display(io);

extern "C"
{
   void app_main();
}
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h>

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
   printf("CalEPD epaper research. Plasticlogic.com test\n");
   /** Color constants that the epaper supports:
    EPD_BLACK 0x00
    EPD_DGRAY 0x01
    EPD_LGRAY 0x02
    EPD_WHITE 0x03
    */
   
   // Initialize display class
   display.init();         // Add init(true) for debug
   display.setRotation(0); // Sets rotation in Adafruit GFX */
   display.clearScreen();
   print_plastic_logic("logic.com", EPD_DGRAY);  // Prints plasticlogic.com
   display.update();
   vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait N seconds
   display.clearScreen();
   display.update();

   // Print temperature
   display.setFont(&Ubuntu_M24pt8b);
   display.setCursor(30,40);
   display.print(display.readTemperatureString(1)); // 1: Appends °C
   display.update();

   //return; // Stop here
   
   vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait N seconds
   display.clearScreen();  // Do a clearScreen between frames otherwise last one will remain and only new parts overwritten
   display.update();

   
   // DEMO for 148 * 72 1.1 inches. Please adapt following variables for different sizes
   uint8_t rectW = 37;

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

   display.fillCircle(rectW, display.height()/2, 15, EPD_LGRAY);
   display.update(); // full update
   vTaskDelay(500 / portTICK_PERIOD_MS);
   display.fillCircle(rectW*2, display.height()/2, 15, EPD_DGRAY);
   display.update(EPD_UPD_PART);
   vTaskDelay(500 / portTICK_PERIOD_MS);
   display.fillCircle(rectW*3, display.height()/2, 15, EPD_BLACK);
   display.update(EPD_UPD_PART);
   vTaskDelay(500 / portTICK_PERIOD_MS);
   display.clearScreen();

   display.fillCircle(rectW*3, display.height()/2, 15, EPD_DGRAY);
   display.update();
   vTaskDelay(300 / portTICK_PERIOD_MS);
   display.fillCircle(rectW*2, display.height()/2, 15, EPD_LGRAY);
   display.update(EPD_UPD_PART);
   vTaskDelay(300 / portTICK_PERIOD_MS);
   display.drawCircle(rectW, display.height()/2, 15, EPD_BLACK);
   display.update(EPD_UPD_PART);
   vTaskDelay(300 / portTICK_PERIOD_MS);
   print_title("end", EPD_DGRAY, 54);
   display.update(EPD_UPD_PART);
}
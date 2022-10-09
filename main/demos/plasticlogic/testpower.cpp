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
//#include <plasticlogic011.h>
//#include <plasticlogic014.h>
#include <plasticlogic021.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
//PlasticLogic011 display(io);
//PlasticLogic014 display(io);
PlasticLogic021 display(io);
bool playShortDemo = true;

extern "C"
{
   void app_main();
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
   display.init();
   display.fillScreen(EPD_BLACK);
   display.drawPixel(10,10,255);
   display.update();

   /* vTaskDelay(2000 / portTICK_PERIOD_MS);
   display.fillScreen(EPD_WHITE);
   display.update(); */

   // TODO: Rotation is not working as it should:
}

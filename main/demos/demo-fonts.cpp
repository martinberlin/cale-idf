#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include <plasticlogic014.h>
EpdSpi2Cs io;
PlasticLogic014 display(io);


// FONT used for title / message body - Only after display library
// Please check https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts#adding-new-fonts-2002831-18
// Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

//#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>


extern "C"
{
   void app_main();
}

void demo_chars(){
 for (int i = 32; i <= 126; i++)
   {
      display.write(i);
   } 
 for (int i = 126+33; i <= 255; i++)
   {
      //printf("%d ",i);
      display.write(i);
   }
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Show available Dynamic Random Access Memory available after display.init()
   printf("Fonts-demo. Free heap: %d (After epaper instantiation)\n", 
   xPortGetFreeHeapSize());

   // Bootstrap epaper class
   display.init(false);
   // Store your epapers all white, just turn true:
   if (false) {
     display.fillScreen(EPD_WHITE); 
     display.update();
     return;
   }


   display.setRotation(2); // 0 - 12.48 w/USB pointing down
   display.fillScreen(EPD_LGRAY);
   display.update();
   vTaskDelay(700); // short delay to demonstrate red color working
   display.fillScreen(EPD_WHITE);

   display.fillCircle(30,30, 10, EPD_LGRAY);

   display.setCursor(1,10);
   display.setTextColor(EPD_BLACK);
   
   display.setFont(&Ubuntu_M8pt8b);

   display.println("German characters test");
   display.println("° äöü ÄÖÜ ß");
   display.println("Löwen, Bären, Vögel und Käfer sind Tiere. Völlerei lässt grüßen! Heute ist 38° zu warm.");
   

   //demo_chars();

   display.update();
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include <gdew042t2.h>
#include <gdew0583t7.h>
#include <gdew075T7.h>
#include <gdew075T8.h>
#include <gdew027w3.h>
//#include <gdeh0213b73.h>
// Single SPI EPD
//EpdSpi io;
//Gdew075T8 display(io);
//Gdew075T7 display(io);
//Gdew042t2 display(io);
//Gdew0583T7 display(io);
//Gdew027w3 display(io);
//Gdeh0213b73 display(io);

// Multi-SPI 4 channels EPD only - 12.48 Epaper display
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
//#include "wave12i48.h" // Only to use with Edp4Spi IO
#include "wave12i48BR.h" // Only to use with Edp4Spi IO, Black Red model
Epd4Spi io;
Wave12I48RB display(io);



// FONT used for title / message body - Only after display library
// Please check https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts#adding-new-fonts-2002831-18
// Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

//#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
//#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
//#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

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
   printf("Fonts-demo. Free heap: %d (After epaper instantiation)\nDRAM     : %d\n", 
   xPortGetFreeHeapSize(),heap_caps_get_free_size(MALLOC_CAP_8BIT));

   // Bootstrap epaper class
   display.init(false);
   // Store your epapers all white, just turn true:
   if (false) {
     display.fillScreen(EPD_RED); 
     display.update();
     return;
   }


   display.setRotation(2); // 0 - 12.48 w/USB pointing down
   display.fillScreen(EPD_BLACK);


   display.setCursor(10,40);
   display.setTextColor(EPD_WHITE);
   
   display.setFont(&Ubuntu_M16pt8b);

   display.println("German characters test");
   display.println("° äöü ÄÖÜ ß");
   display.println("Löwen, Bären, Vögel und Käfer sind Tiere. Völlerei lässt grüßen! Heute ist 38° zu warm.");
   display.println("");
   display.println("Spanish / French characters test");
   display.println("æçèé êëìí ï ñ");
   display.println("La cigüeña estaba sola en la caña. Estás allí?");
   display.newline(); // new way to add a newline
   
   // German sentence
   display.setFont(&Ubuntu_M8pt8b);
   display.print("Ubuntu 8pt");
   demo_chars();

   
   display.println("");
   display.print("\nUbuntu 12pt");
   display.setFont(&Ubuntu_M12pt8b);
   display.setTextColor(EPD_RED);
   demo_chars();

   // Let's draw one 100px radius circle Black and another on the right 120px radius Red
   display.fillCircle(300,300, 100, EPD_BLACK);

   display.fillCircle(600,300, 120, EPD_RED);

   display.update();
}

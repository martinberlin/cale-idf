#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size

//#include <gdew042t2.h>
//#include <gdew0583t7.h>
//#include <gdew075T7.h>
#include <gdew075T8.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>
//#include "wave12i48.h" // Only to use with Edp4Spi IO

// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
//Epd4Spi io;
//Wave12I48 display(io);

// Single SPI EPD
EpdSpi io;
Gdew075T8 display(io);
//Gdew075T7 display(io);
//Gdew042t2 display(io);
//Gdew0583T7 display(io);
//Gdew027w3 display(io);
//Gdeh0213b73 display(io); // Does not work correctly yet - moved to /fix

// FONT used for title / message body - Only after display library
// Please check https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts#adding-new-fonts-2002831-18
// Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

#include <Fonts/TomThumb.h>
#include <Fonts/Ubuntu_M18pt8b.h>

extern "C"
{
   void app_main();
}

void app_main(void)
{
   
   printf("CALE-IDF fonts demo\n");
   // Bootstrap epaper class
   display.init(true);
   display.setRotation(2);
   display.setCursor(10,40);
   display.setTextColor(EPD_BLACK);

   // Please note that TomThumb font is not rendering all character spectrum: 
   // display.setFont(&TomThumb);
   display.setFont(&Ubuntu_M18pt8b);

   for (int i = 32; i <= 255; i++)
   {
      display.write(i); // Needs to be >32 (first character definition)
   }

   display.update();

}

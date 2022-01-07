/**
 * plasticlogic.com Demo of 4 gray level epaper with partial refresh for ESP32 and ESP32S2
 * Proprietry process to manufacture truly flexible, organic thin-film transistors (OTFT)
 * NOTE: This GIF only works partially. Never really come to fix it and only previewable GIFs are:
 * homer  &  homer_tiny
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <AnimatedGIF.h>
// Digit animated GIFs
#include <homer_tiny.h>
#include <anime_sm.h>
#include <badgers.h>
// Replace here the variable name in the GIF.h above
uint8_t * gif_array = (uint8_t *) homer_tiny;
uint32_t gif_array_size = sizeof(homer_tiny);

// Should match with your epaper module and size
// One or many classes can be included at the same time
#include <plasticlogic011.h>
#include <plasticlogic014.h>
#include <plasticlogic021.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic021 display(io);
//PlasticLogic014 display(io);
//PlasticLogic021 display(io);

AnimatedGIF gif; 

extern "C"
{
   void app_main();
}
// IDF function to emulate Arduino's millis()
uint32_t millis() {
   return esp_timer_get_time()/1000;
}

uint8_t color;
long lTime;

uint8_t invertColor(uint8_t color) {
  uint8_t inverted = 3; // 3 is white
  switch (color)
  {
  case 3:
    inverted = 0;
    break;
  case 2:
    inverted = 3;
    break;
  case 1:
    inverted = 2;
    break;
  default:
    inverted = 2;
    break;
  }
  return inverted;
}

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[display.width()];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > display.width())
       iWidth = display.width() - pDraw->iX;
    usPalette = pDraw->pPalette;

    y =  pDraw->iY + pDraw->y; // current line
    if (y >= display.height() || pDraw->iX >= display.width() || iWidth < 1)
       return; 
    s = pDraw->pPixels;

    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }

    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) { // if transparency used
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < iWidth) {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent) { // done, stop
            s--; // back up to treat it like transparent
          }
          else { // opaque
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        
        if (iCount) { // any opaque pixels?
          for (int i=0; i < iCount; i++) {
              uint16_t usColor, usPixel = usTemp[x+i];
              usColor = (usPixel >> 11); // 5 bits of red
              usColor += ((usPixel & 0x7e0) >> 5); // 6 bits of green
              usColor += (usPixel & 0x1f); // 5 bits of blue
              // We now have 7 bits of gray, turn it into 2 bits of gray              
              //printf("%d ",color);
              display.drawPixel( x + i, y, (usColor>>5));
          }
          x += iCount;
          iCount = 0;
        }

        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount) {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    } else { // no transparency
        for (int x=0; x < pDraw->iWidth; x++) {
            uint16_t usColor, usPixel = usTemp[x];
            usColor = (usPixel >> 11); // 5 bits of red
            usColor += ((usPixel & 0x7e0) >> 5); // 6 bits of green
            usColor += (usPixel & 0x1f); // 5 bits of blue
            // We now have 7 bits of gray, turn it into 2 bits of gray
            display.drawPixel( x, y, (usColor>>5));
        }
    }
} /* GIFDraw() */

void app_main(void)
{
   printf("CalEPD version: %s for Plasticlogic.com\nAnimated GIF demo\n", CALEPD_VERSION);
   
   /** Color constants that the epaper supports:
    EPD_BLACK 0x00
    EPD_DGRAY 0x01
    EPD_LGRAY 0x02
    EPD_WHITE 0x03
    */
   
   // Initialize display class
   display.init();         // Add init(true) for debug
  gif.begin(GIF_PALETTE_RGB888);

  int rc;
  rc = gif.open(gif_array, gif_array_size, GIFDraw);
  
  if (rc) {
    while (rc) {
      rc = gif.playFrame(true, NULL); // play a frame and pause for the correct amount of time

      //display.print("wertzuioopasdfghjklm");
      display.update(EPD_UPD_PART);   //EPD_UPD_PART
      vTaskDelay(10 / portTICK_PERIOD_MS);
      display.fillScreen(EPD_WHITE);
    }
    gif.close();
  }
  

}
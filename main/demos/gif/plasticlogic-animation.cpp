/**
 * plasticlogic.com Demo of 4 gray level epaper with partial refresh for ESP32 and ESP32S2
 * Proprietry process to manufacture truly flexible, organic thin-film transistors (OTFT),
 * with industrialization developed in the world’s first commercial, high volume, plastic electronics factory.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <AnimatedGIF.h>
// Digit animated GIFs
#include <homer.h>
#include <anime.h>
// Replace here the variable name in the GIF.h above
uint8_t * gif_array = (uint8_t *) anime;
uint32_t gif_array_size = sizeof(anime);

// Should match with your epaper module and size
// One or many classes can be included at the same time
#include <plasticlogic011.h>
#include <plasticlogic014.h>
#include <plasticlogic021.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
//PlasticLogic011 display(io);
//PlasticLogic014 display(io);
PlasticLogic021 display(io);
bool playShortDemo = true;

AnimatedGIF gif; 

extern "C"
{
   void app_main();
}
// IDF function to emulate Arduino's millis()
uint32_t millis() {
   return esp_timer_get_time()/1000;
}

uint8_t in_blue = 0;
uint8_t in_red = 0;
uint8_t in_green = 0;
uint8_t color;
long lTime;

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

    y =  pDraw->y; // current line
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
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        
        if (iCount) // any opaque pixels?
        {
          
           // This part needs to be adapted for 2bpp framebuffer (PlasticLogic)
          for (uint16_t c=0; c < iWidth; ++c) {
            // It's 888 so we read 3 bytes to get the color:
            for (uint8_t bc=0; bc < 3; ++bc) {
              uint8_t in_byte = usTemp[(3*c)+bc];
              switch (bc)
                {
                case 0:
                    in_blue  = (in_byte>> 3) & 0x1f;
                    break;
                case 1:
                    in_green = ((in_byte >> 2) & 0x3f) << 5;
                    break;
                case 2:
                    in_red   = ((in_byte >> 3) & 0x1f) << 11;
                    break;
                }
                // 255/90 = 2.83
              color = (in_red | in_green | in_blue) /90;
            }
            
            //printf("%d ", color); // W the fuckio!
            display.drawPixel(x, y, color);
          }
          x += iCount;
          iCount = 0;
        }

        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
      
    }
    
}

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
      display.update(EPD_UPD_PART);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      display.fillScreen(EPD_WHITE);
    }
    gif.close();
  }
  

}

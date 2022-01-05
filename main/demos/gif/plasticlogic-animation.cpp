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
#include <digit_0.h>
#include <digit_1.h>
#include <digit_2.h>
#include <digit_3.h>
#include <digit_4.h>
#include <digit_5.h>
#include <digit_6.h>
#include <digit_7.h>
#include <digit_8.h>
#include <digit_9.h>

const uint8_t * digits[10] = {digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7, digit_8, digit_9};
const size_t lengths[10] = {sizeof(digit_0), sizeof(digit_1), sizeof(digit_2), sizeof(digit_3), sizeof(digit_4), sizeof(digit_5), sizeof(digit_6), sizeof(digit_7), sizeof(digit_8), sizeof(digit_9)};

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
// Need 4 independent instances of the class to animate the 4 digits simultaneously
AnimatedGIF gif[4]; 

extern "C"
{
   void app_main();
}

typedef struct my_private_struct
{
  int xoff, yoff; // corner offset
} PRIVATE;

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
    PRIVATE *pPriv = (PRIVATE *)pDraw->pUser;
    
    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > display.width())
       iWidth = display.width() - pDraw->iX;
    usPalette = pDraw->pPalette;
    y = pPriv->yoff + pDraw->iY + pDraw->y; // current line
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
            display.drawPixel(pPriv->xoff + pDraw->iX + x + c, y, color);
          }

          //tft.setAddrWindow(pPriv->xoff + pDraw->iX + x, y, iCount, 1);
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
    else
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<iWidth; x++) {
        usTemp[x] = usPalette[*s++];
        printf("N%d ", usTemp[x]);
        }

      /* tft.startWrite();
      tft.setAddrWindow(pPriv->xoff + pDraw->iX, y, iWidth, 1);
      tft.writePixels(usTemp, iWidth, false, false);
      tft.endWrite(); */
    }
}

//
// Display 4 animated digits (80 pixels wide each)
// only animate the digits which change from one iteration to the next
//
void ShowDigits(int iValue, int iOldValue)
{
int i, rc, iBusy;
PRIVATE priv;
int jn, jo, t0, t1;

  priv.yoff = 72; // center digits vertically  
  iBusy = 0;
  // mark digitis which need to change with a single bit flag
  jn = iValue; jo = iOldValue;
  for (i=3; i>=0; i--) { // compare old and new values
    t0 = jn % 10; t1 = jo % 10;
    if (t0 != t1) {
      iBusy |= (1 << i);
      gif[i].open((uint8_t *)digits[t0], lengths[t0], GIFDraw); // prepare the right digit animated file
    }
    jn /= 10;
    jo /= 10; // next digit
  }
 
  i = 1; // Remove if FOR is enabled
  while (iBusy) {
    // Draw each frame of each changing digit together so that they animate together
    lTime = millis() + 40; // play the frames at a rate of 25fps (40ms per frame)
    //for (i=0; i<4; i++) {
     //  if (iBusy & (1 << i)) {
         // Advance this animation one frame
         priv.xoff = 80 * i; // each digit is 80 pixels wide
         rc = gif[i].playFrame(false, NULL, (void *)&priv); // draw it and return immediately
         if (!rc) { // animation has ended
            iBusy &= ~(1<<i); // clear the bit indicating this digit is busy
            gif[i].close();
    /*      }
       } */
    } // for each digit
    display.update(EPD_UPD_PART);
    display.fillScreen(EPD_WHITE);
    vTaskDelay(150 / portTICK_PERIOD_MS);
  } // while digits still changing
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

   // TODO: Rotation is not working as it should:
   //display.setEpdRotation(2); // 2: Does not turn it portrait, just upside down (Same with Paperino PL_microEPD)
   
   for (int i=0; i<4; i++) {
      // GIF_PALETTE_RGB565_LE (Also another possibility)
     gif[i].begin(GIF_PALETTE_RGB888);
   }

  uint16_t i = 0;
  uint16_t iOld = 9999; // force every digit to be drawn the first time

  ShowDigits(i, iOld);


}

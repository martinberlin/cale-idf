// 7.5 800*480 V2 b/w Manufacturer sample: https://github.com/waveshare/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp
//             Alternative class just for Waveshare 7.5 V2 since partial update was not working as expected
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include <stdint.h>
#include <math.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include <string>
#include <epd.h>
#include <Adafruit_GFX.h>
#include <epdspi.h>
#include "soc/rtc_wdt.h"
#define GDEW075T7_WIDTH 800
#define GDEW075T7_HEIGHT 480

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define GDEW075T7_BUFFER_SIZE (uint32_t(GDEW075T7_WIDTH) * uint32_t(GDEW075T7_HEIGHT) / 8)
// 8 pix of this color in a buffer byte:
#define GDEW075T7_8PIX_BLACK 0xFF
#define GDEW075T7_8PIX_WHITE 0x00

class Wave7i5 : public Epd
{
  public:
   
    Wave7i5(EpdSpi& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug);
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075T7_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
  
};
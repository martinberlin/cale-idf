// Product page: https://www.good-display.com/product/341.html
// 1.02 mono     Controller: UC8175
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
#include <gdew_4grays.h>
#include <esp_timer.h>

#define GDEW0102I3F_WIDTH 80
#define GDEW0102I3F_HEIGHT 128
#define GDEW0102I3F_BUFFER_SIZE (uint32_t(GDEW0102I3F_WIDTH) * uint32_t(GDEW0102I3F_HEIGHT) / 8)

#define GDEW0102I3F_8PIX_BLACK 0x00
#define GDEW0102I3F_8PIX_WHITE 0xFF

class Gdew0102I3F : public Epd
{
  public:
    // & Copy contructor: Copying the injected IO into our class so we can access IO methods
    Gdew0102I3F(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    // EPD tests 
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    void fillScreen(uint16_t color);
    void update();
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    EpdSpi& IO;

    uint8_t _black_buffer[GDEW0102I3F_BUFFER_SIZE];

    bool _initial = true;

    void _wakeUp();
    void _wakeUpPart();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);

    uint8_t lut_w_partial[42] =   {
      0x60, 0x01, 0x01, 0x00, 0x00, 0x01,
      0x80, 0x0f, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    uint8_t lut_b_partial[42] =   {
      0x90, 0x01, 0x01, 0x00, 0x00, 0x01,
      0x40, 0x0f, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
};
// Epaper: 5.65inch ACeP 7-Color  https://www.waveshare.com/product/displays/e-paper/5.65inch-e-paper-module-f.htm
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
#include <epd7color.h>
#include <Adafruit_GFX.h>
#include <epdspi.h>
#include <color/wave7colors.h>
#include <esp_timer.h>

// Controller: Unknown
#define WAVE5I7COLOR_WIDTH 600
#define WAVE5I7COLOR_HEIGHT 448
#define WAVE5I7COLOR_BUFFER_SIZE (uint32_t(WAVE5I7COLOR_WIDTH) * uint32_t(WAVE5I7COLOR_HEIGHT) / 2)
#define IS_COLOR_EPD true

class Wave5i7Color : public Epd7Color
{
  public:
   
    Wave5i7Color(EpdSpi& IO);
    const uint8_t colors_supported = 7;
    bool spi_optimized = true;
    const bool has_partial_update = false;
    
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[WAVE5I7COLOR_BUFFER_SIZE];

    bool _initial = true;
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};
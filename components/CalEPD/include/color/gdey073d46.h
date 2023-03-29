// Epaper: ACeP 7-Color GOODISPLAY https://www.good-display.com/product/442.html
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
#define GDEY073D46_WIDTH 800
#define GDEY073D46_HEIGHT 480
#define GDEY073D46_BUFFER_SIZE (uint32_t(GDEY073D46_WIDTH) * uint32_t(GDEY073D46_HEIGHT) / 2)
#define IS_COLOR_EPD true

class gdey073d46 : public Epd7Color
{
  public:
    gdey073d46(EpdSpi& IO);
    const uint8_t colors_supported = 7;
    bool spi_optimized = false;
    const bool has_partial_update = false;
    
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;
    // In case this _buffer is too large and there is no DRAM available to build, then store it in PSRAM
    //uint8_t _buffer[GDEY073D46_BUFFER_SIZE];
    uint8_t* _buffer = (uint8_t*)heap_caps_malloc(GDEY073D46_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};

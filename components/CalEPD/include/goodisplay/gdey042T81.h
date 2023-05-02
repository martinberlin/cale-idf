// 4.2 b/w GDEY042T81 https://www.good-display.com/product/386.html
// Note from GOODISPLAY: The GDEQ042T81 is fully compatible with GDEY042T81
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
#include <gdew_colors.h>
#include <esp_timer.h>

// Controller: SSD1683

#define GDEY042T81_WIDTH 400
#define GDEY042T81_HEIGHT 300
#define GDEY042T81_BUFFER_SIZE (uint32_t(GDEY042T81_WIDTH) * uint32_t(GDEY042T81_HEIGHT) / 8)

#define GDEY042T81_8PIX_BLACK 0x00
#define GDEY042T81_8PIX_WHITE 0xFF

class Gdey042T81 : public Epd
{
  public:
   
    Gdey042T81(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    uint8_t fastmode = 0; // With 1 it will not go to power off
    bool is_powered = false;
    
    // EPD tests 
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    void fillScreen(uint16_t color);
    void update();
    void deepsleep();

  private:
    EpdSpi& IO;

    uint8_t _black_buffer[GDEY042T81_BUFFER_SIZE];

    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    static const epd_init_4 epd_resolution;
};
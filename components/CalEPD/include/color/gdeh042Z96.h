// 4.2 b/w/red
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

// Controller: SSD1619A https://v4.cecdn.yun300.cn/100001_1909185148/SSD1619A.pdf

#define GDEH042Z96_WIDTH 400
#define GDEH042Z96_HEIGHT 300
#define GDEH042Z96_BUFFER_SIZE (uint32_t(GDEH042Z96_WIDTH) * uint32_t(GDEH042Z96_HEIGHT) / 8)
#define IS_COLOR_EPD true
#define GDEH042Z96_8PIX_BLACK 0x00
#define GDEH042Z96_8PIX_WHITE 0xFF
#define GDEH042Z96_8PIX_RED 0xFF
#define GDEH042Z96_8PIX_RED_WHITE 0x00

// Note: GDEW0213I5F is our test display that will be the default initializing this class
class Gdeh042Z96 : public Epd
{
  public:
   
    Gdeh042Z96(EpdSpi& IO);
    uint8_t colors_supported = 3;
    
    // EPD tests 
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _black_buffer[GDEH042Z96_BUFFER_SIZE];
    uint8_t _red_buffer[GDEH042Z96_BUFFER_SIZE];

    bool _initial = true;
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    static const epd_init_4 epd_resolution;
};
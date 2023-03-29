// Product page: https://www.good-display.com/product/395.html
// 4.2 b/w/red   Controller: UC8276
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

#define GDEH042Z21_WIDTH 400
#define GDEH042Z21_HEIGHT 300
#define GDEH042Z21_BUFFER_SIZE (uint32_t(GDEH042Z21_WIDTH) * uint32_t(GDEH042Z21_HEIGHT) / 8)
#define IS_COLOR_EPD true
#define GDEH042Z21_8PIX_BLACK 0x00
#define GDEH042Z21_8PIX_WHITE 0xFF
#define GDEH042Z21_8PIX_RED 0x00
#define GDEH042Z21_8PIX_RED_WHITE 0xFF 

class Gdeq042Z21 : public Epd
{
  public:
    // & Copy contructor: Copying the injected IO into our class so we can access IO methods
    Gdeq042Z21(EpdSpi& IO);
    uint8_t colors_supported = 3;
    
    // EPD tests 
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    void fillScreen(uint16_t color);
    void update();
    void setModel(std::string set_model);
    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);
    std::string model = "GDEH042Z15";

  private:
    EpdSpi& IO;

    uint8_t _black_buffer[GDEH042Z21_BUFFER_SIZE];
    uint8_t _red_buffer[GDEH042Z21_BUFFER_SIZE];

    bool _initial = true;

    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};
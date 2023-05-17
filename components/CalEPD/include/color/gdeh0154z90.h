// Contributed by thebino: https://github.com/martinberlin/cale-idf/issues/47
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

// Controller: SSD1681  Note: Used by Waveshare in https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_(B)
#define GDEH0154Z90_WIDTH 200
#define GDEH0154Z90_HEIGHT 200
#define GDEH0154Z90_BUFFER_SIZE (uint32_t(GDEH0154Z90_WIDTH) * uint32_t(GDEH0154Z90_HEIGHT) / 8)
#define IS_COLOR_EPD true
#define GDEH0154Z90_8PIX_BLACK 0x00
#define GDEH0154Z90_8PIX_WHITE 0xFF
#define GDEH0154Z90_8PIX_RED 0xFF
#define GDEH0154Z90_8PIX_RED_WHITE 0x00

class Gdeh0154z90 : public Epd
{
public:
    Gdeh0154z90(EpdSpi &IO);
    uint8_t colors_supported = 2;
    uint8_t partial_update_supported = 0;

    void init(bool debug = false);
    void fillScreen(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color); // Override GFX own drawPixel method

    void update();

private:
    EpdSpi &IO;

    uint8_t _black_buffer[GDEH0154Z90_BUFFER_SIZE];
    uint8_t _red_buffer[GDEH0154Z90_BUFFER_SIZE];

    bool _initial = true;

    void _wakeUp();
    void _sleep();
    void _waitBusy(const char *message);

    void _rotate(int16_t &x, int16_t &y, int16_t &w, int16_t &h);
};

// GOODISPLAY product https://www.good-display.com/product/389.html
// Controller:        SSD1680
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

// All comments below are from J-M Zingg (Ref. EPD)
// The physical number of pixels (for controller parameter)
#define GDEY029T94_X_PIXELS 128
#define GDEY029T94_Y_PIXELS 296

// The logical width and height of the display
#define GDEY029T94_WIDTH 128
#define GDEY029T94_HEIGHT 296

// Note: the visible number of display pixels is 122*250, see GDEH0213B72 V1.1 Specification.pdf
#define GDEY029T94_VISIBLE_WIDTH 128

#define GDEY029T94_BUFFER_SIZE (uint32_t(GDEY029T94_WIDTH) * uint32_t(GDEY029T94_HEIGHT) / 8)

class Gdey029T94 : public Epd
{
  public:
    Gdey029T94(EpdSpi& IO);
    // Counts only Ink color so BWR will have 2
    const uint8_t colors_supported = 1;
    const uint8_t partial_supported = 1;
    uint8_t spi_optimized = true;

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void setMonoMode(bool mode);
    void fillScreen(uint16_t color);
    void update();
    void eraseDisplay(bool using_partial_update = false);

    // Partial update test status please check repository Wiki
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
  
  private:
    EpdSpi& IO;
    bool _mono_mode = false;
    uint8_t _mono_buffer[GDEY029T94_BUFFER_SIZE];
    uint8_t _buffer1[GDEY029T94_BUFFER_SIZE];
    uint8_t _buffer2[GDEY029T94_BUFFER_SIZE];

    bool debug_enabled = false;
    
    void _wakeUp();
    void _wakeUpGrayMode();
    void _sleep();

    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    // Ram data entry mode methods
    void _setRamDataEntryMode(uint8_t em);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    // Command & data structs
    // LUT tables for this display are filled with zeroes at the end with writeLuts()
    static const epd_lut_159 lut_4_grays;
};

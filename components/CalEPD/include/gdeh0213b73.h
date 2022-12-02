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

// Controller: SSD1675B 
// All comments below are from J-M Zingg (Ref. EPD)
// The physical number of pixels (for controller parameter)
#define GDEH0213B73_X_PIXELS 128
#define GDEH0213B73_Y_PIXELS 250

// The logical width and height of the display
#define GDEH0213B73_WIDTH GDEH0213B73_X_PIXELS
#define GDEH0213B73_HEIGHT GDEH0213B73_Y_PIXELS

// Note: the visible number of display pixels is 122*250, see GDEH0213B72 V1.1 Specification.pdf
#define GDEH0213B73_VISIBLE_WIDTH 122

#define GDEH0213B73_BUFFER_SIZE (uint32_t(GDEH0213B73_WIDTH) * uint32_t(GDEH0213B73_HEIGHT) / 8)

class Gdeh0213b73 : public Epd
{
  public:
    Gdeh0213b73(EpdSpi& IO);
    // Sorry manufacturers but I will count on black/white only real ink colors supported not white (Others will respect your color number definition)
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();

    void fillScreen(uint16_t color);
    void update();
    void eraseDisplay(bool using_partial_update = false);

    // Partial update test status please check repository Wiki
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // Partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEH0213B73_BUFFER_SIZE];

    bool debug_enabled = false;
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _powerOn();
    void _wakeUp();
    void _sleep();
    void cmd(uint8_t command);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    // Ram data entry mode methods
    void _setRamDataEntryMode(uint8_t em);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    // Command & data structs
    static const epd_lut_100 lut_data_full;
    static const epd_lut_100 lut_data_part;
};
#ifndef CALEPD_EXCLUDE_PARALLEL

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
#include <epdParallel.h>
#include <Adafruit_GFX.h>
#include <epdspi.h>
#include "epd_driver.h"
#include "epd_highlevel.h"
#include "parallel/grayscales.h"

#define HAS_16_LEVELS_GRAY 1
#define ED060SC4_WIDTH 800
#define ED060SC4_HEIGHT 600

class Ed060SC4 : public EpdParallel
{
  public:
    Ed060SC4();
    EpdiyHighlevelState hl;
    uint8_t *framebuffer;
    uint8_t colors_supported = 1;

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method

    void init(bool debug = false);
    void clearScreen();
    void clearArea(EpdRect area);
    void powerOn();
    void powerOff();
    
    void fillScreen(uint16_t color);
    void update(enum EpdDrawMode mode = MODE_GC16);

    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, enum EpdDrawMode mode = MODE_EPDIY_BLACK_TO_GL16, bool using_rotation = true);

  private:
    bool _tempalert = false;
    bool _initial = true;
    bool _debug_buffer = false;
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};

#endif

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
#include "L58Touch.h" // Touch interface
#include "parallel/grayscales.h"

#define HAS_16_LEVELS_GRAY 1
#define ED047TC1_WIDTH 960
#define ED047TC1_HEIGHT 540


class Ed047TC1t : public EpdParallel
{
  public:
    Ed047TC1t(L58Touch& ts);
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
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, enum EpdDrawMode mode = MODE_GC16);

    // Touch methods
    void touchLoop();
    void registerTouchHandler(void(*fn)(TPoint point, TEvent e));
    void registerMultiTouchHandler(void(*fn)(TPoint point1, TPoint point2, TEvent e));
    void(*_touchHandler)(TPoint point, TEvent e) = nullptr;
    void displayRotation(uint8_t rotation); // Rotates both Epd & Touch

  private:
    L58Touch& Touch;
    
    bool color = false;
    bool _initial = true;
    bool _debug_buffer = false;
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};

#endif

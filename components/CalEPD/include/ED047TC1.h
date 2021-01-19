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
#include <gdew_colors.h>
#include "epd_driver.h"

// Controller: IL3829
#define ED047TC1_WIDTH 960
#define ED047TC1_HEIGHT 540

#define ED047TC1_BUFFER_SIZE (uint32_t(ED047TC1_WIDTH) * uint32_t(ED047TC1_HEIGHT) / 8)
// 1 byte of this color in the buffer
// This needs to be refactored since this display uses 4 bit per pixel

class Ed047TC1 : public EpdParallel
{
  public:
    Ed047TC1();

    uint8_t *framebuffer;
    uint8_t colors_supported = 1;

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method

    void init(bool debug = false);
    void clearScreen();
    void clearArea(Rect_t area);
    void powerOn();
    void powerOff();
    
    //void fillScreen(uint16_t color); // Not implemented
    void update(); //uint8_t *framebuffer
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    uint8_t _buffer[ED047TC1_BUFFER_SIZE];
    
    bool color = false;
    bool _initial = true;
    bool _debug_buffer = false;
};
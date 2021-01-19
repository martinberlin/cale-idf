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
#include "epd_driver.h"

// Controller: IL3829
#define ED047TC1_WIDTH 960
#define ED047TC1_HEIGHT 540

#define ED047TC1_BUFFER_SIZE (uint32_t(ED047TC1_WIDTH) * uint32_t(ED047TC1_HEIGHT) / 8)
// 1 byte of this color in the buffer
// This needs to be refactored since this display uses 4 bit per pixel

class Ed047TC1 : public Epd
{
  public:
    Ed047TC1();
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();
    void clearScreen();
    
    // Not implemented
    //void fillScreen(uint16_t color);
    void update();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    uint8_t _buffer[ED047TC1_BUFFER_SIZE];
    bool color = false;
    bool _initial = true;
    bool _debug_buffer = false;
    void _PowerOn();
    void _writeCommandData(const uint8_t cmd, const uint8_t* pCommandData, uint8_t datalen); // Waits for busy on each command

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();

    void _wakeUp(uint8_t em);
    void _sleep();

    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    // Command & data structs
    static const epd_init_30 LUTDefault_full;
    static const epd_init_30 LUTDefault_part;
    static const epd_init_3 GDOControl;
};
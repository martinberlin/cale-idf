// 12.48" 1304*984 b/w Controller: ???
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
#include <epd4spi.h>
#include "soc/rtc_wdt.h"       // Watchdog control the dogs
#define WAVE12I48_WIDTH 1304
#define WAVE12I48_HEIGHT 984

// Not sure if this will work, since the 4 displays share the buffer
// Ideally only the first one could work for GFX and the rest just to fill with a bitmap
#define WAVE12I48_BUFFER_SIZE (uint32_t(WAVE12I48_WIDTH) * uint32_t(WAVE12I48_HEIGHT) / 8)

// 50 % of the screen only GFX, just a test, since a 160Kb Buffer won't work
//#define WAVE12I48_BUFFER_SIZE (uint32_t(WAVE12I48_WIDTH) * uint32_t(WAVE12I48_HEIGHT) / 16)

class Wave12I48 : public Epd
{
  public:
   
    Wave12I48(Epd4Spi& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug);
    void initFullUpdate();
    void initPartialUpdate();
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();

  private:
    Epd4Spi& IO;

    uint8_t _buffer[WAVE12I48_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    
    // Command & data structs
    // LUT tables for this display are filled with zeroes at the end with writeLuts()
    static const epd_init_42 lut_20_LUTC_partial;
    
    static const epd_power_4 epd_wakeup_power;
    static const epd_init_1 epd_panel_setting_full;
    static const epd_init_1 epd_panel_setting_partial;
    static const epd_init_1 epd_pll;
    static const epd_init_4 epd_resolution;
};
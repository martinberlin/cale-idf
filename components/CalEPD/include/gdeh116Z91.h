// 11.6 960*640 b/red Controller: SSD1677
// ATC version, not sure if is the same controller: https://twitter.com/atc1441/status/1327586319510499328
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
#include "soc/rtc_wdt.h"
#include <gdew_colors.h>

#define GDEH116Z91_WIDTH 960
#define GDEH116Z91_HEIGHT 640


#define GDEH116Z91_BUFFER_SIZE (uint32_t(GDEH116Z91_WIDTH) * uint32_t(GDEH116Z91_HEIGHT) / 8)
// 8 pix of this color in a buffer byte:
#define GDEH116Z91_8PIX_BLACK 0x00
#define GDEH116Z91_8PIX_WHITE 0xFF

class Gdeh116Z91 : public Epd
{
  public:
   
    Gdeh116Z91(EpdSpi& IO);
    uint8_t colors_supported = 3;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    void init(bool debug = false);
    void initFullUpdate();
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEH116Z91_BUFFER_SIZE];
    uint8_t _color[GDEH116Z91_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    void _wakeUp();
    void _sleep();
    void _full_update();
    void _waitBusy(const char* message);
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    
    // Initialization commands
    static const epd_init_5 soft_start;
};
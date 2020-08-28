// 5.83 600*448 b/w/R Controller: IL0371 (3 colors) http://www.e-paper-display.com/download_detail/downloadsId%3d536.html
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

#define GDEW075C64_WIDTH 800
#define GDEW075C64_HEIGHT 480

#define GDEW075C64_BUFFER_SIZE (uint32_t(GDEW075C64_WIDTH) * uint32_t(GDEW075C64_HEIGHT) / 8)

#define GDEW075C64_PU_DELAY 500

class Gdew075C64 : public Epd
{
  public:
   
    Gdew075C64(EpdSpi& IO);
    uint8_t colors_supported = 3;
    
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillScreen(uint16_t color);
    void update();
    // Color epapers from Goodisplay do not support partial update
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075C64_BUFFER_SIZE];
    uint8_t _color_buffer[GDEW075C64_BUFFER_SIZE]; // Yellow or Red depending on the model
    bool _using_partial_mode = false;
    bool _initial = true;
    
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void _send8pixel(uint8_t black,uint8_t red);

    // Command & data structs
    static const epd_init_4 epd_wakeup_power;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_3 epd_boost;
    static const epd_init_4 epd_resolution;
};
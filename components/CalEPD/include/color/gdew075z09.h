// 5.83 640*380 b/w/R Controller: IL0371 (3 colors) http://www.e-paper-display.com/download_detail/downloadsId%3d536.html
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
// Note in S3 rtc_wdt has errors: https://github.com/espressif/esp-idf/issues/8038
#if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
  #include "soc/rtc_wdt.h"
#endif
#include <gdew_colors.h>
#include <esp_timer.h>

#define GDEW075Z09_WIDTH 640
#define GDEW075Z09_HEIGHT 384
#define IS_COLOR_EPD true
#define GDEW075Z09_BUFFER_SIZE (uint32_t(GDEW075Z09_WIDTH) * uint32_t(GDEW075Z09_HEIGHT) / 8)

#define GDEW075Z09_PU_DELAY 480

class Gdew075z09 : public Epd
{
  public:
   
    Gdew075z09(EpdSpi& IO);
    uint8_t colors_supported = 3;
    
    void init(bool debug = false);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillScreen(uint16_t color);
    void update();
    // Color epapers from Goodisplay do not support partial update
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075Z09_BUFFER_SIZE];
    uint8_t _red_buffer[GDEW075Z09_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void _send8pixel(uint8_t black,uint8_t red);
    // Command & data structs
   
    static const epd_init_2 epd_wakeup_power;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_3 epd_boost;
    static const epd_init_4 epd_resolution;
};
// 5.83 600*448 b/w Controller: IL0371
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

#define GDEW0583T7_WIDTH 600
#define GDEW0583T7_HEIGHT 448

#define GDEW0583T7_BUFFER_SIZE (uint32_t(GDEW0583T7_WIDTH) * uint32_t(GDEW0583T7_HEIGHT) / 8)

#define GDEW0583T7_PU_DELAY 500

class Gdew0583T7 : public Epd
{
  public:
   
    Gdew0583T7(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void init(bool debug = false);
    void eraseDisplay(bool using_partial_update);
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();
    
    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW0583T7_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void _send8pixel(uint8_t data);
    // Command & data structs
    // LUT tables for this display are filled with zeroes at the end with writeLuts()
    static const epd_init_42 lut_20_LUTC_partial;
    static const epd_init_42 lut_21_LUTWW_partial;
    static const epd_init_42 lut_22_LUTKW_partial;
    static const epd_init_42 lut_23_LUTWK_partial;
    static const epd_init_42 lut_24_LUTKK_partial;
    static const epd_init_42 lut_25_LUTBD_partial;
    
    static const epd_init_2 epd_wakeup_power;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_3 epd_boost;
    static const epd_init_1 epd_panel_setting_partial;
    static const epd_init_1 epd_pll;
    static const epd_init_1 epd_temperature;
    static const epd_init_4 epd_resolution;
};
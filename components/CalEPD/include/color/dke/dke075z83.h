// 7.5 800*480 b/w/R DKE: UC8179 Product link: https://www.dke.top/products/fast-refresh-no-flicker-880x528-big-75-inch-epaper-display-e-ink-screene-ink-display
// Name is like this because it was the first and used same controler as 5.83z83 from GoodDisplay. Real model is: DEPG0750BNU790F32
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

#define DKE075Z83_WIDTH 800
#define DKE075Z83_HEIGHT 480
#define IS_COLOR_EPD true
#define DKE075Z83_BUFFER_SIZE (uint32_t(DKE075Z83_WIDTH) * uint32_t(DKE075Z83_HEIGHT) / 8)
// 8 pix definitions
#define DKE075Z83_8PIX_BLACK 0x00
#define DKE075Z83_8PIX_WHITE 0xFF
#define DKE075Z83_8PIX_RED 0x00
#define DKE075Z83_8PIX_RED_WHITE 0xFF

#define DKE075Z83_PU_DELAY 500

class Dke075Z83 : public Epd
{
  public:
   
    Dke075Z83(EpdSpi& IO);
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

    uint8_t _black_buffer[DKE075Z83_BUFFER_SIZE];
    uint8_t _red_buffer[DKE075Z83_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    
    // Command & data structs
   
    static const epd_init_4 epd_wakeup_power;
    static const epd_init_1 epd_panel_setting;
    static const epd_init_4 epd_resolution;
};
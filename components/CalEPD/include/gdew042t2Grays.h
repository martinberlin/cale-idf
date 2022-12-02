// 4.2 b/w H
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
#if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
  #include "soc/rtc_wdt.h"
#endif
#include <gdew_4grays.h>
#include <esp_timer.h>
// Controller: UC8176
#define GDEW042T2_WIDTH 400
#define GDEW042T2_HEIGHT 300

// 1 bit per pixel monochrome
#define GDEW042T2_MONO_BUFFER_SIZE (uint32_t(GDEW042T2_WIDTH) * uint32_t(GDEW042T2_HEIGHT) / 8)

// Note: GDEW0213I5F is our test display that will be the default initializing this class
class Gdew042t2Grays : public Epd
{
  public:
   
    Gdew042t2Grays(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void setMonoMode(bool mode);
    void initFullUpdate();
    void initPartialUpdate();
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    void fillScreen(uint16_t color);
    void update();

    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;
    bool _mono_mode = false;
    uint8_t _buffer1[GDEW042T2_MONO_BUFFER_SIZE];
    uint8_t _buffer2[GDEW042T2_MONO_BUFFER_SIZE];
    uint8_t _mono_buffer[GDEW042T2_MONO_BUFFER_SIZE];
    bool _initial = true;
    bool _partial_mode = false;
    //uint16_t _partials = 0;
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    // Command & data structs - Mono?
    static const epd_init_30 lut_full; // 4 gray

    static const epd_init_42 lut_vcom11;
    static const epd_init_42 lut_ww_full;
    static const epd_init_42 lut_bw_full;
    static const epd_init_42 lut_wb_full;
    static const epd_init_42 lut_bb_full;
    
    static const epd_init_44 lut_20_vcom0_partial;
    static const epd_init_42 lut_21_ww_partial;
    static const epd_init_42 lut_22_bw_partial;
    static const epd_init_42 lut_23_wb_partial;
    static const epd_init_42 lut_24_bb_partial;

    static const epd_power_4 epd_wakeup_power;
    static const epd_init_3 epd_soft_start;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_1 epd_pll;
    static const epd_init_4 epd_resolution;
};
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
#include <gdew_colors.h>
#include <esp_timer.h>
// Controller: IL0398 (old version, new see Grays class) : http://www.good-display.com/download_detail/downloadsId=537.html

#define GDEW042T2_WIDTH 400
#define GDEW042T2_HEIGHT 300
#define GDEW042T2_BUFFER_SIZE (uint32_t(GDEW042T2_WIDTH) * uint32_t(GDEW042T2_HEIGHT) / 8)

#define GDEW042T2_8PIX_BLACK 0x00
#define GDEW042T2_8PIX_WHITE 0xFF

// Note: GDEW0213I5F is our test display that will be the default initializing this class
class Gdew042t2 : public Epd
{
  public:
   
    Gdew042t2(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    void fillScreen(uint16_t color);
    void update();

    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW042T2_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    // Command & data structs
    static const epd_init_44 lut_vcom0_full;
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
    static const epd_init_1 epd_panel_setting;
    static const epd_init_1 epd_pll;
    static const epd_init_4 epd_resolution;
};
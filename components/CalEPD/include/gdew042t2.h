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
// Controller: IL0398
#define GxGDEW042T2_WIDTH 400
#define GxGDEW042T2_HEIGHT 300
#define GxGDEW042T2_BUFFER_SIZE (uint32_t(GxGDEW042T2_WIDTH) * uint32_t(GxGDEW042T2_HEIGHT) / 8)


// Note: GxGDEW0213I5F is our test display that will be the default initializing this class
class Gdew042t2 : public Epd
{
  public:
   
    Gdew042t2(EpdSpi& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug);
    void initFullUpdate();

    void fillScreen(uint16_t color);
    void update();

    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GxGDEW042T2_BUFFER_SIZE];

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    // Command & data structs
    static const epd_init_44 lut_20_vcomDC;
    static const epd_init_42 lut_21_ww;
    static const epd_init_42 lut_22_bw;
    static const epd_init_42 lut_23_wb;
    static const epd_init_42 lut_24_bb;

    static const epd_init_44 lut_20_vcomDC_partial;
    static const epd_init_42 lut_21_ww_partial;
    static const epd_init_42 lut_22_bw_partial;
    static const epd_init_42 lut_23_wb_partial;
    static const epd_init_42 lut_24_bb_partial;

    static const epd_power_5 epd_wakeup_power;
    static const epd_init_3 epd_soft_start;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_1 epd_pll;
    static const epd_init_3 epd_resolution;
};
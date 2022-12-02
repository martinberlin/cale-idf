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

// Controller: IL0373
#define GDEW0213I5F_WIDTH 104
#define GDEW0213I5F_HEIGHT 212
#define GDEW0213I5F_BUFFER_SIZE (uint32_t(GDEW0213I5F_WIDTH) * uint32_t(GDEW0213I5F_HEIGHT) / 8)


// Note: GDEW0213I5F is our test display that will be the default initializing this class
class Gdew0213i5f : public Epd
{
  public:
   
    Gdew0213i5f(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();

    void fillScreen(uint16_t color);
    void update();
    void eraseDisplay(bool using_partial_update = false);

    // Both partial updates DO NOT work as expected, turning all screen black or making strange effects
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // Partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);

    // This are already inherited from Epd: write(uint8_t); print(const std::string& text);println(same);

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW0213I5F_BUFFER_SIZE];

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

    static const epd_init_5 epd_wakeup_power;
    static const epd_init_3 epd_soft_start;
    static const epd_init_2 epd_panel_setting;
    static const epd_init_1 epd_pll;
    static const epd_init_3 epd_resolution;
};
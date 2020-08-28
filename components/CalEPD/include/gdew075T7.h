// 7.5 800*480 b/w Controller: GD7965 (In Waveshare called 7.5 V2)
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

#define GDEW075T7_WIDTH 800
#define GDEW075T7_HEIGHT 480

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define GDEW075T7_BUFFER_SIZE (uint32_t(GDEW075T7_WIDTH) * uint32_t(GDEW075T7_HEIGHT) / 8)
// 8 pix of this color in a buffer byte:
#define GDEW075T7_8PIX_BLACK 0x00
#define GDEW075T7_8PIX_WHITE 0xFF

class Gdew075T7 : public Epd
{
  public:
   
    Gdew075T7(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    // Alternative method to updateWindow
    // Partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075T7_BUFFER_SIZE];
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
    static const epd_init_42 lut_21_LUTWW_partial;
    static const epd_init_42 lut_22_LUTKW_partial;
    static const epd_init_42 lut_23_LUTWK_partial;
    static const epd_init_42 lut_24_LUTKK_partial;
    static const epd_init_42 lut_25_LUTBD_partial;
    
    static const epd_power_4 epd_wakeup_power;
    static const epd_init_1 epd_panel_setting_full;
    static const epd_init_1 epd_panel_setting_partial;
    static const epd_init_1 epd_pll;
    static const epd_init_4 epd_resolution;
};
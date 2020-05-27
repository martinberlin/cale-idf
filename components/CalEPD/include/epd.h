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
#include <Adafruit_GFX.h>
#include <espspi.h>

// the only colors supported by any of these displays; mapping of other colors is class specific
#define GxEPD_BLACK     0x0000
#define GxEPD_DARKGREY  0x7BEF      /* 128, 128, 128 */
#define GxEPD_LIGHTGREY 0xC618      /* 192, 192, 192 */
#define GxEPD_WHITE     0xFFFF
#define GxEPD_RED       0xF800      /* 255,   0,   0 */

#define GxGDEW0213I5F_WIDTH 104
#define GxGDEW0213I5F_HEIGHT 212
#define GxGDEW0213I5F_BUFFER_SIZE (uint32_t(GxGDEW0213I5F_WIDTH) * uint32_t(GxGDEW0213I5F_HEIGHT) / 8)
// divisor for AVR, should be factor of GxGDEW0213I5F_HEIGHT
#define GxGDEW0213I5F_PAGES 4
#define GxGDEW0213I5F_PAGE_HEIGHT (GxGDEW0213I5F_HEIGHT / GxGDEW0213I5F_PAGES)
#define GxGDEW0213I5F_PAGE_SIZE (GxGDEW0213I5F_BUFFER_SIZE / GxGDEW0213I5F_PAGES)

typedef struct {
    uint8_t cmd;
    uint8_t data[42];
    uint8_t databytes; //No of data in data; 0xFF = end of cmds.
} epd_init_42;

typedef struct {
    uint8_t cmd;
    uint8_t data[44];
    uint8_t databytes;
} epd_init_44;

typedef struct {
    uint8_t cmd;
    uint8_t data[1];
} epd_init_1;

typedef struct {
    uint8_t cmd;
    uint8_t data[2];
} epd_init_2;

typedef struct {
    uint8_t cmd;
    uint8_t data[3];
} epd_init_3;

typedef struct {
    uint8_t cmd;
    uint8_t data[5];
} epd_power_5;

// Note: GxGDEW0213I5F is our test display that will be the default initializing this class
class Epd : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd driver";
    
    Epd(EspSpi& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug);
    void initFullUpdate();
    void initPartialUpdate();

    void fillScreen(uint16_t color);
    void update();
    void eraseDisplay(bool using_partial_update = false);

    // Both partial updates DO NOT work as expected, turning all screen black or making strange effects
    // partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);

    // hook to Adafruit_GFX::write
    size_t write(uint8_t);
    void print(const std::string& text);
    void println(const std::string& text);

  protected: 
    // This should be inherited from abstract class so we don't repeat in every model
    static inline uint16_t gx_uint16_min(uint16_t a, uint16_t b) {return (a < b ? a : b);};
    static inline uint16_t gx_uint16_max(uint16_t a, uint16_t b) {return (a > b ? a : b);};

  private:
    EspSpi& IO;
    uint8_t _buffer[GxGDEW0213I5F_BUFFER_SIZE];
    // Very smart template from GxEPD to swap x,y:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    bool _using_partial_mode = false;
    bool debug_enabled = true;

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
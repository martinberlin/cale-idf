// 7.5 800*480 b/w Controller: UC8179

// IMPORTANT: Do not use this with epapers Waveshare labelled V2. Those use GD7965 (Check Gdew075 for that)
// PREVIOUS Note: Controller seems also fully compatible with UC8179C -> Not true, wakeUp is different, lines test fails
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

#define GDEY075T7_WIDTH 800
#define GDEY075T7_HEIGHT 480

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define GDEY075T7_BUFFER_SIZE (uint32_t(GDEY075T7_WIDTH) * uint32_t(GDEY075T7_HEIGHT) / 8)
// 8 pix of this color in a buffer byte:
#define GDEY075T7_8PIX_BLACK 0x00
#define GDEY075T7_8PIX_WHITE 0xFF

class Gdey075T7 : public Epd
{
  public:
   
    Gdey075T7(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests
    void init(bool debug = false);

    void initPartialUpdate();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void fillRawBufferPos(uint16_t index, uint8_t value);
    void fillRawBufferImage(uint8_t image[], uint16_t size);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEY075T7_BUFFER_SIZE];
    // Place _buffer in external RAM
    //uint8_t* _buffer = (uint8_t*)heap_caps_malloc(GDEY075T7_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

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
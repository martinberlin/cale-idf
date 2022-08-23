// 7.5 HD 880*528 b/w Controller: ??
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

#define GDEW075HD_WIDTH 880
#define GDEW075HD_HEIGHT 528

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define GDEW075HD_BUFFER_SIZE (uint32_t(GDEW075HD_WIDTH) * uint32_t(GDEW075HD_HEIGHT) / 8)
// 8 pix of this color in a buffer byte:
#define GDEW075HD_8PIX_BLACK 0x00
#define GDEW075HD_8PIX_WHITE 0xFF

class Gdew075HD : public Epd
{
  public:
   
    Gdew075HD(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);

    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075HD_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};
// 7.5 640*384 b/w Controller: IL0371
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

#define GDEW075T8_WIDTH 640
#define GDEW075T8_HEIGHT 384

// 8 pix of this color in a buffer byte:
#define GDEW075T8_8PIX_BLACK 0xFF
#define GDEW075T8_8PIX_WHITE 0x00

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define GDEW075T8_BUFFER_SIZE (uint32_t(GDEW075T8_WIDTH) * uint32_t(GDEW075T8_HEIGHT) / 8)

class Gdew075T8 : public Epd
{
  public:
   
    Gdew075T8(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);
  
    void init(bool debug = false);
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void eraseDisplay(bool using_partial_update);
    void fillScreen(uint16_t color);
    void update();

  private:
    EpdSpi& IO;

    uint8_t _buffer[GDEW075T8_BUFFER_SIZE];
    bool _using_partial_mode = false;
    bool _initial = true;
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void _send8pixel(uint8_t data);
    void _send8pixelPack(uint8_t data);
};
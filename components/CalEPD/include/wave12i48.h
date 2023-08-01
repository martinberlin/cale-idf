// 12.48" 1304*984 b/w Controller: ???
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
#include <epd4spi.h>
// Note in S3 rtc_wdt has errors: https://github.com/espressif/esp-idf/issues/8038
#if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
  #include "soc/rtc_wdt.h"       // Watchdog control
#endif
#include <gdew_colors.h>
#include <esp_timer.h>

#define WAVE12I48_WIDTH 1304
#define WAVE12I48_HEIGHT 984

// 8 pix of this color in a buffer byte:
#define WAVE12I48_8PIX_BLACK 0x00
#define WAVE12I48_8PIX_WHITE 0xFF

// Buffer size in bytes
#define WAVE12I48_BUFFER_SIZE (uint32_t(WAVE12I48_WIDTH) * uint32_t(WAVE12I48_HEIGHT) / 8)

#define WAVE_BUSY_TIMEOUT 2000000

class Wave12I48 : public Epd
{
  public:
   
    Wave12I48(Epd4Spi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    // EPD tests 
    void init(bool debug = false);
    void clear();
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void update();

  private:
    Epd4Spi& IO;

    //uint8_t _buffer[WAVE12I48_BUFFER_SIZE];
    uint8_t* _buffer = (uint8_t*)heap_caps_malloc(WAVE12I48_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

    bool _initial = true;
    
    void _wakeUp();
    void _powerOn();
    void _sleep();
    void _waitBusy(const char* message);
    void _waitBusyM1(const char* message);
    void _waitBusyM2(const char* message);
    void _waitBusyS1(const char* message);
    void _waitBusyS2(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    
    // Command & data structs
    static const epd_init_1 epd_panel_setting_full;
    static const epd_init_4 epd_resolution_m1s2;
    static const epd_init_4 epd_resolution_m2s1;
};

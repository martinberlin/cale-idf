// 7.5 800*480 b/w Controller: GD7965 (In Waveshare called 7.5 V2)
// Please note: This buffer requires PSRAM
// idf.py menuconfig
// → Component config → ESP32-specific
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

#define GDEW075T7_WIDTH 800
#define GDEW075T7_HEIGHT 480
#define EPD_LGRAY 220
#define EPD_DGRAY 40

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
#define GDEW075T7_BUFFER_SIZE (uint32_t(GDEW075T7_WIDTH) * uint32_t(GDEW075T7_HEIGHT) / 2)

class Gdew075T7Grays : public Epd
{
  public:
   
    Gdew075T7Grays(EpdSpi& IO);
    uint8_t colors_supported = 1;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    
    void test4bit();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation);
    void fillScreen(uint16_t color);
    void sendLuts();
    void fillRawBufferPos(uint32_t index, uint8_t value);
    void fillRawBufferImage(uint8_t *image, uint32_t size);
    void update();

  private:
    EpdSpi& IO;
    uint8_t* _buffer = (uint8_t*)heap_caps_malloc(GDEW075T7_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

    bool _initial = true;
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    
    // Command & data structs
    // LUT tables for this display are filled with zeroes at the end with writeLuts()
    static const epd_init_42 lut_vcom;
    static const epd_init_42 lut_ww;
    static const epd_init_42 lut_bw;
    static const epd_init_42 lut_wb;
    static const epd_init_42 lut_bb;
    
    static const epd_power_4 epd_wakeup_power;
    static const epd_init_1 epd_panel_setting_full;
    static const epd_init_1 epd_pll;
    static const epd_init_4 epd_resolution;

};
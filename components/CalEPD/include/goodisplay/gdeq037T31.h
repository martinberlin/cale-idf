// GOODISPLAY product page still not online
// Controller:        UC8253
// GDEQ037T31_416x240
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
#include <gdew_4grays.h>
#include <esp_timer.h>

// Display dimensions
#define GDEQ037T31_WIDTH  240
#define GDEQ037T31_HEIGHT 416

#define GDEQ037T31_BUFFER_SIZE (uint32_t(GDEQ037T31_WIDTH) * uint32_t(GDEQ037T31_HEIGHT) / 8)

// 1 byte of this color in the buffer
#define GDEQ037T31_8PIX_BLACK 0xFF
#define GDEQ037T31_8PIX_WHITE 0x00

class Gdeq037T31 : public Epd
{
  public:
    Gdeq037T31(EpdSpi& IO);
    bool fast_mode = false;

    // Counts only Ink color so BWR will have 2
    const uint8_t colors_supported = 1;
    const uint8_t partial_supported = 1;
    uint16_t total_updates = 0;
    bool spi_optimized = true;

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();
    void setMonoMode(bool mode); // No idea if supports 4 grays
    void fillScreen(uint16_t color);
    void update();
    
    // Partial update test status please check repository Wiki
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
  
  private:
    EpdSpi& IO;
    bool _mono_mode = false;
    uint8_t _mono_buffer[GDEQ037T31_BUFFER_SIZE];
    uint8_t _old_buffer[GDEQ037T31_BUFFER_SIZE];
    bool debug_enabled = false;
    
    void _wakeUp();
    void _sleep();
    
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    // Ram data entry mode methods
    void _setRamDataEntryMode(uint8_t em);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);

};

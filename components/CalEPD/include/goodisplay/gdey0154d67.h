// GOODISPLAY product https://www.good-display.com/product/388.html
//            can be used alond DESPI-C03 adapter with Front light 
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

// Controller: SSD1681 (Acts similar than IL3829)
#define GDEY0154D67_WIDTH 200
#define GDEY0154D67_HEIGHT 200
#define GDEY0154D67_BUFFER_SIZE (uint32_t(GDEY0154D67_WIDTH) * uint32_t(GDEY0154D67_HEIGHT) / 8)
// 1 byte of this color in the buffer
#define GDEY0154D67_8PIX_BLACK 0xFF
#define GDEY0154D67_8PIX_WHITE 0x00

class Gdey0154d67 : public Epd
{
  public:
    Gdey0154d67(EpdSpi& IO);

    const uint8_t colors_supported = 1;
    const uint8_t partial_supported = 1;
    bool spi_optimized = true;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void setMonoMode(bool mode);
    void fillScreen(uint16_t color);
    void update();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

  private:
    EpdSpi& IO;

    uint8_t _mono_buffer[GDEY0154D67_BUFFER_SIZE];
    uint8_t _buffer1[GDEY0154D67_BUFFER_SIZE];
    uint8_t _buffer2[GDEY0154D67_BUFFER_SIZE];
    
    bool _initial = true;
    bool _mono_mode = true;

    void _PowerOn();
    void _setRamDataEntryMode(uint8_t em);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);

    // Default wakeUp used for 4 gray mode
    void _wakeUp();

    void _wakeUp(uint8_t em);
    void _sleep();
    void _waitBusy(const char* message, uint16_t busy_time);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

    // Command & data structs
    static const epd_lut_159 lut_4_grays;
    static const epd_init_3 GDOControl;
};

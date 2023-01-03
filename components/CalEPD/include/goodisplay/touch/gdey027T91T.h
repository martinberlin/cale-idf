// GOODISPLAY product https://www.good-display.com/product/432.html
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
#include "FT6X36.h" // Touch interface

// Controller: SSD1680 (Solomon)
#define GDEY027T91T_WIDTH 176
#define GDEY027T91T_HEIGHT 264
#define GDEY027T91T_BUFFER_SIZE (uint32_t(GDEY027T91T_WIDTH) * uint32_t(GDEY027T91T_HEIGHT) / 8)
// 1 byte of this color in the buffer
#define GDEY027T91T_8PIX_BLACK 0xFF
#define GDEY027T91T_8PIX_WHITE 0x00

class Gdey027T91T : public Epd
{
  public:
    Gdey027T91T(EpdSpi& IO, FT6X36& ts);

    const uint8_t colors_supported = 1;
    const uint8_t partial_supported = 1;
    bool spi_optimized = true;

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void initFullUpdate();
    void initPartialUpdate();
    void setMonoMode(bool mode);
    void fillScreen(uint16_t color);
    void update();
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // TOUCH related
    void displayRotation(uint8_t rotation); // Rotates both Epd & Touch
    void touchLoop();
    void registerTouchHandler(void(*fn)(TPoint point, TEvent e));
    void(*_touchHandler)(TPoint point, TEvent e) = nullptr;

  private:
    EpdSpi& IO;
    FT6X36& Touch;

    uint8_t _mono_buffer[GDEY027T91T_BUFFER_SIZE];
    uint8_t _buffer1[GDEY027T91T_BUFFER_SIZE];
    uint8_t _buffer2[GDEY027T91T_BUFFER_SIZE];

    bool color = false;
    bool _initial = true;
    bool _debug_buffer = false;
    bool _mono_mode = false;
    
    void _PowerOn();
    // Ram data entry mode methods
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

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
#define DEPG750BN_WIDTH 800
#define DEPG750BN_HEIGHT 480
#define DEPG750BN_BUFFER_SIZE DEPG750BN_WIDTH * DEPG750BN_HEIGHT/8
// 1 byte of this color in the buffer
#define DEPG750BN_8PIX_BLACK 0xFF
#define DEPG750BN_8PIX_WHITE 0x00

class Depg750bn : public Epd
{
  public:
	Depg750bn(EpdSpi& IO);

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

    uint8_t _mono_buffer[DEPG750BN_BUFFER_SIZE];

    bool _initial = true;
    bool _mono_mode = true;
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _setRamDataEntryMode(uint8_t em);
    void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
    void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
    void _Init_PartialUpdate(void);

    void _wakeUp();
    void _PowerOn(void);
    void _wakeUp(uint8_t em);
    void _sleep();
    void _waitBusy(const char* message, uint16_t busy_time);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    void cmd(uint8_t command);
    // Command & data structs

    static const unsigned char lut_25_LUTBD_partial[];
    static const uint8_t LUTDefault_VCOM[];
    static const uint8_t LUTDefault_LUTWW[];
    static const uint8_t LUTDefault_LUTBW[];
    static const uint8_t LUTDefault_LUTWB[];
    static const uint8_t LUTDefault_LUTBB[];
};

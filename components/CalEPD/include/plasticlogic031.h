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
#include <plasticlogic.h>
#include <Adafruit_GFX.h>
#include <plasticlogic.h>
#include <epdspi2cs.h>

// Controller: UC8156  Manufacturer: https://www.plasticlogic.com/products/displays/displays-with-ultrachip/1-1-inch-display
#define PLOGIC031_WIDTH 312
#define PLOGIC031_HEIGHT 76
// TODO: Should be 2 bits per pixel: 
#define PLOGIC031_BUFFER_SIZE (uint32_t(PLOGIC031_WIDTH) * uint32_t(PLOGIC031_HEIGHT) / 4)

class PlasticLogic031 : public PlasticLogic
{
  public:
    PlasticLogic031(EpdSpi2Cs& IO);
    ~PlasticLogic031();
    void init(bool debug = false);
    void clearScreen();
    void update(uint8_t updateMode=EPD_UPD_FULL);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method

  private:
    EpdSpi2Cs& IO;
    uint8_t _buffer[PLOGIC031_BUFFER_SIZE];
    // Buffer sent to EPD prefixed with 0x10:
    uint8_t bufferEpd[PLOGIC031_BUFFER_SIZE+1];

    bool _initial = true;
    bool _debug_buffer = false;
    uint16_t _nextline = PLOGIC031_WIDTH/4;

    // Done for 21 and 31 size
    void scrambleBuffer();
    int _getPixel(int x, int y);
};
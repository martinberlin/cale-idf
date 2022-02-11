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
#define PLOGIC021_WIDTH 240
#define PLOGIC021_HEIGHT 146
// TODO: Should be 2 bits per pixel: 
#define PLOGIC021_BUFFER_SIZE (uint32_t(PLOGIC021_WIDTH) * uint32_t(PLOGIC021_HEIGHT) / 4)

class PlasticLogic021 : public PlasticLogic
{
  public:
    PlasticLogic021(EpdSpi2Cs& IO);
    ~PlasticLogic021();
    void init(bool debug = false);
    void clearScreen();
    void update(uint8_t updateMode=EPD_UPD_FULL);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    void setEpdRotation(uint8_t o); // Override

    uint8_t _buffer[PLOGIC021_BUFFER_SIZE];
  private:
    EpdSpi2Cs& IO;
    // Buffer sent to EPD prefixed with 0x10:
    uint8_t bufferEpd[PLOGIC021_BUFFER_SIZE+1];

    bool _initial = true;
    bool _debug_buffer = false;
    uint16_t _nextline = PLOGIC021_WIDTH/4;

    // Done for 21 and 31 size
    void scrambleBuffer();
    int _getPixel(int x, int y);
};
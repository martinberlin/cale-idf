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

// Controller: UC8156  Manufacturer: https://www.plasticlogic.com/products/displays
#define PLOGIC014_WIDTH 180
#define PLOGIC014_HEIGHT 100

#define PLOGIC014_BUFFER_SIZE (uint32_t(PLOGIC014_WIDTH) * uint32_t(PLOGIC014_HEIGHT) / 4)

class PlasticLogic014 : public PlasticLogic
{
  public:
    PlasticLogic014(EpdSpi2Cs& IO);
    ~PlasticLogic014();
    void init(bool debug = false);
    void clearScreen();
    void update(uint8_t updateMode=EPD_UPD_FULL);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

  private:
    EpdSpi2Cs& IO;
    uint8_t _buffer[PLOGIC014_BUFFER_SIZE];
    // Buffer sent to EPD prefixed with 0x10:
    uint8_t bufferEpd[PLOGIC014_BUFFER_SIZE+1];

    bool _initial = true;
    bool _debug_buffer = false;
    uint16_t _nextline = PLOGIC014_WIDTH/4;
};
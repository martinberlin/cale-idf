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
#define PLOGIC011_WIDTH 148
#define PLOGIC011_HEIGHT 72
// TODO: Should be 2 bits per pixel: 
#define PLOGIC011_BUFFER_SIZE (uint32_t(PLOGIC011_WIDTH) * uint32_t(PLOGIC011_HEIGHT) / 4)

class PlasticLogic011 : public PlasticLogic
{
  public:
    PlasticLogic011(EpdSpi2Cs& IO);
    
    void init(bool debug = false);
    void clearScreen();
    void update(uint8_t updateMode=EPD_UPD_FULL);
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method

    // Internal temperature sensor
    uint8_t readTemperature();
    std::string readTemperatureString(uint8_t type = 0); // 0: string 1: celsius
    uint8_t getEPDsize();
    // Bosch Accelerometer BMA250E not readable still. Check plastic/accelerometer branch
    void  setEpdRotation(uint8_t o);
    void _powerOn();
    void _powerOff();

  private:
    EpdSpi2Cs& IO;
    uint8_t _buffer[PLOGIC011_BUFFER_SIZE];
    // Buffer sent to EPD prefixed with 0x10:
    uint8_t bufferEpd[PLOGIC011_BUFFER_SIZE+1];

    bool _initial = true;
    bool _debug_buffer = false;
    uint16_t _nextline = PLOGIC011_WIDTH/4;

    // Accelerometer
    int x, y, z, temp;

    void _wakeUp();

    void _sleep();
    void _waitBusy(const char* message, uint16_t busy_time);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};
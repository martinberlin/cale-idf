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
#include <plasticlogic.h>
#include <epdspi2cs.h>

// Controller: UC8156  Manufacturer: https://www.plasticlogic.com/products/displays/displays-with-ultrachip/1-1-inch-display
#define PLOGIC011_WIDTH 148
#define PLOGIC011_HEIGHT 72
// TODO: Should be 2 bits per pixel: 
#define PLOGIC011_BUFFER_SIZE (uint32_t(PLOGIC011_WIDTH) * uint32_t(PLOGIC011_HEIGHT) / 4)

class PlasticLogic011 : public Epd
{
  public:
    PlasticLogic011(EpdSpi2Cs& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    void clearScreen(); 
    void update();
    void update(uint8_t updateMode);

    // Bosch Accelerometer BMA250E
    void accelBegin();
    void accelActivateTapOnInt1();
    void accelClearLatchedInt1();
    void accelReadAccel();
    void accelDeepSuspend();
    void csStateToogle(const char* message);

    uint8_t getEPDsize();

    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    
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

    // Accelerometer + temperature (Not on EPD but on the Paperino SPI interface PCB)
    int x, y, z, temp;

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();

    void _sleep();
    void _waitBusy(const char* message, uint16_t busy_time);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
};
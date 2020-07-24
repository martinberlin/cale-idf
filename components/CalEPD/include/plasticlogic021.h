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
#define PLOGIC021_WIDTH 240
#define PLOGIC021_HEIGHT 146
// TODO: Should be /4 according to plasticlogic: 
#define PLOGIC021_BUFFER_SIZE (uint32_t(PLOGIC021_WIDTH) * uint32_t(PLOGIC021_HEIGHT) / 4)
// 1 byte of this color in the buffer (Not sure if I need this for this EPD)
#define PLOGIC021_8PIX_BLACK 0xFF
#define PLOGIC021_8PIX_WHITE 0x00

class PlasticLogic021 : public Epd
{
  public:
    PlasticLogic021(EpdSpi2Cs& IO);
    
    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug);

    void fillScreen(uint16_t color);
    void update();
    void update(uint8_t updateMode);

    // Bosch Accelerometer BMA250E
    void accelBegin();
    void accelActivateTapOnInt1();
    void accelClearLatchedInt1();
    void accelReadAccel();
    void accelDeepSuspend();

    uint8_t getEPDsize();

    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    
    void  setRotation(uint8_t o);
    void _powerOn();
    void _powerOff();

  private:
    EpdSpi2Cs& IO;
    uint8_t _buffer[PLOGIC021_BUFFER_SIZE];
    bool color = false;
    bool _initial = true;
    bool _debug_buffer = false;
    // Accelerometer + temperature
    int x, y, z, temp;

    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _wakeUp();

    void _wakeUp(uint8_t em);
    void _sleep();
    void _waitBusy(const char* message, uint16_t busy_time);
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);

};
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
#include <epdspi2cs.h>
// Controller: UC8156  Manufacturer: https://www.plasticlogic.com/products/displays/displays-with-ultrachip/1-1-inch-display
#define PLOGIC011_WIDTH 148
#define PLOGIC011_HEIGHT 70
#define PLOGIC011_BUFFER_SIZE (uint32_t(PLOGIC011_WIDTH) * uint32_t(PLOGIC011_HEIGHT) / 8)
// 1 byte of this color in the buffer (Not sure if I need this for this EPD)
#define PLOGIC011_8PIX_BLACK 0xFF
#define PLOGIC011_8PIX_WHITE 0x00

// Original defines in https://github.com/RobPo/Paperino
//#define EPD_BLACK 0x00 // Already defined
#define EPD_DGRAY 0x01
#define EPD_LGRAY 0x02
#define EPD_WHITE 0x03

#define EPD_UPD_FULL  0x00            // Triggers a Full update, 4 GL, 800ms
#define EPD_UPD_PART  0x01            // Triggers a Partial update, 4 GL, 800ms
#define EPD_UPD_MONO  0x02            // Triggers a Partial Mono update, 2 GL, 250ms

#define EPD_TMG_LNG     880             // Duration{ms} of a full update
#define EPD_TMG_MID     340             // Duration{ms} of a partial update
#define EPD_TMG_SRT     1
#define EPD_TMG_SR2     70

#define EPD_REVISION          0x00  // Revision, Read only
#define EPD_PANELSETTING      0x01
#define EPD_DRIVERVOLTAGE     0x02
#define EPD_POWERCONTROL      0x03
#define EPD_BOOSTSETTING    	0x04
#define EPD_TCOMTIMING        0x06
#define EPD_INTTEMPERATURE    0x07
#define EPD_SETRESOLUTION     0x0C
#define EPD_WRITEPXRECTSET    0x0D
#define EPD_PIXELACESSPOS     0x0E
#define EPD_DATENTRYMODE      0x0F
#define EPD_DISPLAYENGINE     0x14
#define EPD_VCOMCONFIG        0x18
#define EPD_BORDERSETTING     0x1D
#define EPD_POWERSEQUENCE     0x1F
#define EPD_SOFTWARERESET     0x20
#define EPD_PROGRAMMTP        0x40
#define EPD_MTPADDRESSSETTING 0x41
#define EPD_LOADMONOWF        0x44

#define ACC_GSEL    0x03    // Range: 0x03 - +/-2g, 0x05 - +/-4g, 0x08 - +/-8g, 0x0C - +/-16g
#define ACC_BW      0x0F    // Bandwidth: 0x08 = 7.81Hz bandwith, 0x0F = 1000Hz

class PlasticLogic011 : public Epd
{
  public:
    PlasticLogic011(EpdSpi2Cs& IO);
    
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

    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);

    void _powerOn();
    void _powerOff();

  private:
    EpdSpi2Cs& IO;
    uint8_t _buffer[PLOGIC011_BUFFER_SIZE];
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
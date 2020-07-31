#include <calepd_version.h>
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
#include <Adafruit_GFX.h>
#include <epdspi.h>

#ifndef plasticlogic_h
#define plasticlogic_h
// Original defines in https://github.com/RobPo/Paperino
#define EPD_BLACK 0x00
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
#define EPD_BOOSTSETTING      0x04
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

// Note: GDEW0213I5F is our test display that will be the default initializing this class
class PlasticLogic : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd PlasticLogic driver";
    
    PlasticLogic(int16_t w, int16_t h) : Adafruit_GFX(w,h) {
        printf("CalEPD component version %s\n",CALEPD_VERSION);
    };

    // Every display model should implement this public methods
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;  // Override GFX own drawPixel method
    virtual void init(bool debug = false) = 0;
    virtual void update() = 0; 

    // Partial methods are going to be implemented by each model clases
    //virtual void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true);
    // partial update of rectangle at (xs,ys) from buffer to screen at (xd,yd), does not power off
    //virtual void updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation = true);

    // This are common methods every MODELX will inherit
    // hook to Adafruit_GFX::write
    size_t write(uint8_t);
    void print(const std::string& text);
    void println(const std::string& text);
    void newline();
  // Methods that should be accesible by inheriting this abstract class
  protected: 
    // This should be inherited from this abstract class so we don't repeat in every model
    static inline uint16_t gx_uint16_min(uint16_t a, uint16_t b) {return (a < b ? a : b);};
    static inline uint16_t gx_uint16_max(uint16_t a, uint16_t b) {return (a > b ? a : b);};
    bool _using_partial_mode = false;
    bool debug_enabled = true;
    // Very smart template from EPD to swap x,y:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }

  private:
    // Every display model should implement this private methods
    virtual uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) = 0;
    virtual void _wakeUp() = 0;
    virtual void _sleep() = 0;
    virtual void _waitBusy(const char* message) = 0;
    virtual void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h) = 0;
    uint8_t _unicodePerChar(uint8_t c);
    uint8_t _unicodeEasy(uint8_t c);
};
#endif
#ifndef epd_h
#define epd_h

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
#include <color/wave7colors.h>

// Note: This is the base to inherit for 7 color epapers
class Epd7Color : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd driver 7col";
    
    Epd7Color(int16_t w, int16_t h) : Adafruit_GFX(w,h) {
        printf("CalEPD component version %s\n",CALEPD_VERSION);
    };

    // Every display model should implement this public methods
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;  // Override GFX own drawPixel method
    virtual void init(bool debug = false) = 0;
    virtual void update() = 0; 

    // This are common methods every MODELX will inherit
    // hook to Adafruit_GFX::write
    size_t write(uint8_t);
    void print(const std::string& text);
    void print(const char c);
    void println(const std::string& text);
    void newline();

  // Methods that should be accesible by inheriting this abstract class
  protected: 
     bool debug_enabled = true;
    // Very smart template from EPD to swap x,y:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    uint8_t _color7(uint16_t color);

  private:
    virtual void _wakeUp() = 0;
    virtual void _sleep() = 0;
    virtual void _waitBusy(const char* message) = 0;
    virtual void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h) = 0;
    uint8_t _unicodePerChar(uint8_t c);
    uint8_t _unicodeEasy(uint8_t c);
    // Command & data structs should be implemented by every MODELX display
};
#endif

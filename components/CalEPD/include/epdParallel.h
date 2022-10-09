#pragma once

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

class EpdParallel : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd driver I2S DataBus";
    
    EpdParallel(int16_t w, int16_t h) : Adafruit_GFX(w,h) {
        printf("CalEPD component version %s\n",CALEPD_VERSION);
    };

    // Every display model should implement this public methods
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;  // Override GFX own drawPixel method
    virtual void init(bool debug = false) = 0;
    
    virtual void powerOn() = 0;
    virtual void powerOff() = 0;

    // This are common methods every MODELX will inherit
    // hook to Adafruit_GFX::write
    size_t write(uint8_t);
    void print(const std::string& text);
    void print(const char c);
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
    uint8_t _unicodePerChar(uint8_t c);
    uint8_t _unicodeEasy(uint8_t c);
    // Command & data structs should be implemented by every MODELX display
};

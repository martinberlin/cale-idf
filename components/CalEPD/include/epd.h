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

typedef struct {
    uint8_t cmd;
    uint8_t data[100];
    uint8_t databytes;
} epd_lut_100;

typedef struct {
    uint8_t cmd;
    uint8_t data[42];
    uint8_t databytes; //No of data in data; 0xFF = end of cmds.
} epd_init_42;

typedef struct {
    uint8_t cmd;
    uint8_t data[44];
    uint8_t databytes;
} epd_init_44;

typedef struct {
    uint8_t cmd;
    uint8_t data[30];
    uint8_t databytes;
} epd_init_30;

typedef struct {
    uint8_t cmd;
    uint8_t data[12];
    uint8_t databytes;
} epd_init_12;

typedef struct {
    uint8_t cmd;
    uint8_t data[1];
    uint8_t databytes;
} epd_init_1;

typedef struct {
    uint8_t cmd;
    uint8_t data[2];
    uint8_t databytes;
} epd_init_2;

typedef struct {
    uint8_t cmd;
    uint8_t data[3];
    uint8_t databytes;
} epd_init_3;

typedef struct {
    uint8_t cmd;
    uint8_t data[4];
    uint8_t databytes;
} epd_init_4;

typedef struct {
    uint8_t cmd;
    uint8_t data[5];
    uint8_t databytes;
} epd_init_5;

typedef struct {
    uint8_t cmd;
    uint8_t data[6];
    uint8_t databytes;
} epd_init_6;

typedef struct {
    uint8_t cmd;
    uint8_t data[5];
    uint8_t databytes;
} epd_power_4;


// Note: GDEW0213I5F is our test display that will be the default initializing this class
class Epd : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd driver";
    
    Epd(int16_t w, int16_t h) : Adafruit_GFX(w,h) {
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
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    virtual void _wakeUp() = 0;
    virtual void _sleep() = 0;
    virtual void _waitBusy(const char* message) = 0;
    virtual void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h) = 0;
    uint8_t _unicodePerChar(uint8_t c);
    uint8_t _unicodeEasy(uint8_t c);
    // Command & data structs should be implemented by every MODELX display
};
#endif

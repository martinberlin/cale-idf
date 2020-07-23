#include "plasticlogic011.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Partial Update Delay
#define PLOGIC011_PU_DELAY 100

// Constructor
PlasticLogic011::PlasticLogic011(EpdSpi2Cs& dio): 
  Adafruit_GFX(PLOGIC011_WIDTH, PLOGIC011_HEIGHT),
  Epd(PLOGIC011_WIDTH, PLOGIC011_HEIGHT), IO(dio)
{
  printf("PlasticLogic011() %d*%d\n",
  PLOGIC011_WIDTH, PLOGIC011_HEIGHT);  
}

//Initialize the display
void PlasticLogic011::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("PlasticLogic011::init(%d)\n", debug);
    IO.init(4, debug); // 4MHz frequency

    IO.cmd(EPD_SOFTWARERESET);
    printf("Free heap:%d\n",xPortGetFreeHeapSize());

    IO.csStateLow();
    IO.data(EPD_PANELSETTING);
    IO.data(0x12);
    IO.csStateToogle();

    IO.data(EPD_WRITEPXRECTSET);
    IO.data(0x0);
    IO.data(71);
    IO.data(0x0);
    IO.data(147);
    IO.csStateToogle();

    IO.data(EPD_VCOMCONFIG);
    IO.data(0x0);
    IO.data(0x0);
    IO.data(0x24);
    IO.data(0x07);
    IO.csStateToogle();

    IO.data(EPD_DRIVERVOLTAGE);
    IO.data(0x25);
    IO.data(0xff);
    IO.csStateToogle();

    IO.data(EPD_BORDERSETTING);
    IO.data(0x04);
    IO.csStateToogle();

    IO.data(EPD_LOADMONOWF);
    IO.data(0x60);
    IO.csStateToogle();

    IO.data(EPD_INTTEMPERATURE);
    IO.data(0x0a);
    IO.csStateToogle();

    IO.data(EPD_BOOSTSETTING);
    IO.data(0x22);
    IO.data(0x17);
    IO.csStateHigh();
    //fillScreen(EPD_WHITE);
}

void PlasticLogic011::fillScreen(uint16_t color)
{
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  uint8_t data = (color == EPD_BLACK) ? PLOGIC011_8PIX_BLACK : PLOGIC011_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

uint16_t PlasticLogic011::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  printf("_setPartialRamArea not used in PlasticLogic011");
  return 0;
}
void PlasticLogic011::_wakeUp(){
  printf("_wakeUp not used in PlasticLogic011");
}

void PlasticLogic011::_wakeUp(uint8_t em){
  printf("wakeup() start commands\n");
}

void PlasticLogic011::update(){
  update(EPD_UPD_FULL);
}

void PlasticLogic011::update(uint8_t updateMode)
{
  
  // EPD_PIXELACESSPOS
  printf("BUFF Size:%d\n",sizeof(_buffer));
  IO.csStateLow();
  IO.data(EPD_PIXELACESSPOS);
  IO.data(0);
  IO.data(147);
  IO.csStateToogle();

  // Send buffer
  for (int i=0; i < sizeof(_buffer); i++) {
    IO.data(_buffer[i]);
  }
  
  IO.csStateHigh();
  _waitBusy("_Update_Full", EPD_TMG_SRT);

    switch (updateMode) {
        case 0:
          IO.csStateLow();
          IO.data(EPD_PROGRAMMTP);
          IO.data(0x00);

          IO.csStateToogle();
          IO.data(EPD_DISPLAYENGINE);
          IO.data(0x03);
          IO.csStateHigh();

          _waitBusy("EPD_UPD_FULL", EPD_TMG_LNG);
            break;
        case 1:
            // TODO
            //writeRegister(EPD_PROGRAMMTP, 0x00, -1, -1, -1);
            //writeRegister(EPD_DISPLAYENGINE, 0x07, -1, -1, -1);
            _waitBusy("EPD_UPD_PART", EPD_TMG_LNG);
            break;
        case 2:
            // TODO
            //writeRegister(EPD_PROGRAMMTP, 0x02, -1, -1, -1);
            //writeRegister(EPD_DISPLAYENGINE, 0x07, -1, -1, -1);
            _waitBusy("EPD_UPD_MONO", EPD_TMG_MID);
    }

  _sleep();
}

void PlasticLogic011::_powerOn(void) {
  IO.csStateLow();
  IO.data(EPD_SETRESOLUTION);
  IO.data(0x00);
  IO.data(239);
  IO.data(0x00);
  IO.data(147);
  IO.csStateToogle();

  IO.data(EPD_TCOMTIMING);
  IO.data(0x67);
  IO.data(0x55);
  IO.csStateToogle();
  
  IO.data(EPD_POWERSEQUENCE);
  IO.data(0x00);
  IO.data(0x00);
  IO.data(0x00);
  IO.csStateToogle();

  IO.data(EPD_POWERCONTROL);
  IO.data(0xD1);
  IO.csStateHigh();
  _waitBusy("_PowerOn");
}

void PlasticLogic011::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  
  printf("Not implemented\n");
  vTaskDelay(PLOGIC011_PU_DELAY/portTICK_RATE_MS); 
}

void PlasticLogic011::_waitBusy(const char* message, uint16_t busy_time){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // On high is busy
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  } else {
    vTaskDelay(busy_time/portTICK_RATE_MS); 
  }
}

void PlasticLogic011::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    // On low is not busy anymore
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

// Called _poweroff in microEPD
void PlasticLogic011::_powerOff(){
  IO.csStateLow();
  IO.data(EPD_POWERCONTROL); 
  IO.data(0xD0);            // power off display
  IO.csStateHigh();

  _waitBusy("power_off");
  IO.csStateLow();
  IO.data(EPD_POWERCONTROL);
  IO.data(0xC0);
  IO.csStateHigh();
}

/**
 * Putting the UC8156 in deep sleep mode with less than 1µA current @3.3V
 * Reset pin toggling needed to wakeup the driver IC again.
 */ 
void PlasticLogic011::_sleep(){
  IO.csStateLow();
  IO.data(0x21);  // deepsleep
  IO.data(0xff);          
  IO.data(0xff);
  IO.data(0xff);
  IO.data(0xff);
  IO.cs2StateHigh();
}

void PlasticLogic011::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = PLOGIC011_WIDTH - x - w - 1;
      break;
    case 2:
      x = PLOGIC011_WIDTH - x - w - 1;
      y = PLOGIC011_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = PLOGIC011_HEIGHT - y - h - 1;
      break;
  }
}


void PlasticLogic011::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = PLOGIC011_WIDTH - x - 1;
      break;
    case 2:
      x = PLOGIC011_WIDTH - x - 1;
      y = PLOGIC011_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = PLOGIC011_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * PLOGIC011_WIDTH / 8;

  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
}

    void PlasticLogic011::accelBegin() {
      uint8_t accGsel[2] = {0x0F, ACC_GSEL};
      uint8_t accBand[2] = {0x10, ACC_BW};
      IO.cmdAccel(accGsel, sizeof(accGsel));
      IO.cmdAccel(accBand, sizeof(accBand));
    }
    
    void PlasticLogic011::accelActivateTapOnInt1() {
      uint8_t accEnSingleTap[2]   = {0x16, 0x20};
      uint8_t accMapSingleTap[2]  = {0x19, 0x20};
      uint8_t accAdjTapSens[2]    = {0x2B, 0x01};
      uint8_t accInterruptMode[2] = {0x21, 0x0F};
      IO.cmdAccel(accEnSingleTap, sizeof(accEnSingleTap));
      IO.cmdAccel(accMapSingleTap, sizeof(accMapSingleTap));
      IO.cmdAccel(accAdjTapSens, sizeof(accAdjTapSens));
      IO.cmdAccel(accInterruptMode, sizeof(accInterruptMode));
    }

    void PlasticLogic011::accelClearLatchedInt1() {
      uint8_t accLatch[2] = {0x21, 0x80};
      IO.cmdAccel(accLatch, sizeof(accLatch));
    }

    void PlasticLogic011::accelReadAccel() {
      // Research how to read from SPI (MISO gpio?)
    }

    void PlasticLogic011::accelDeepSuspend() {
      uint8_t accelDeepSuspend[2] = {0x11, 0x20};
      IO.cmdAccel(accelDeepSuspend, sizeof(accelDeepSuspend));
    }
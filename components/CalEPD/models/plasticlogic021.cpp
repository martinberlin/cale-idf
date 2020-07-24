#include "plasticlogic021.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Partial Update Delay
#define PLOGIC021_PU_DELAY 100

// Constructor
PlasticLogic021::PlasticLogic021(EpdSpi2Cs& dio): 
  Adafruit_GFX(PLOGIC021_WIDTH, PLOGIC021_HEIGHT),
  Epd(PLOGIC021_WIDTH, PLOGIC021_HEIGHT), IO(dio)
{
  printf("PlasticLogic021() %d*%d\n",
  PLOGIC021_WIDTH, PLOGIC021_HEIGHT);  
}

//Initialize the display
void PlasticLogic021::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) {
      printf("PlasticLogic021::init(%d) bufferSize: %d width: %d height: %d\n", 
    debug, PLOGIC021_BUFFER_SIZE, PLOGIC021_WIDTH, PLOGIC021_HEIGHT);
    }
    IO.init(4, debug); // 4MHz frequency
    
    IO.reset(10);
    //IO.cmd(EPD_SOFTWARERESET); // No need if rst is defined
    
    uint8_t size = getEPDsize();
    printf("EPD size: %d Free heap:%d\n", size, xPortGetFreeHeapSize());

    IO.csStateLow();
    IO.data(EPD_PANELSETTING);
    IO.data(0x10);
    IO.csStateToogle();

    IO.data(EPD_WRITEPXRECTSET);
    IO.data(0x00);
    IO.data(0xEF); // 239
    IO.data(0x00);
    IO.data(0x91); // 145
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
    
    printf("begin() ends\n");

    //Set landscape mode as default
    setRotation(1);
    /* fillScreen(EPD_WHITE);
    */
    update(); 
}

uint8_t PlasticLogic021::getEPDsize() {
  uint8_t size = 0;
   IO.csStateLow();
   IO.data(EPD_PROGRAMMTP);
   IO.data(0x02);
   IO.data(EPD_MTPADDRESSSETTING);
   IO.data(0xF2);
   IO.data(0x04);
   // Read 1 dummy bytes
   IO.readRegister(0x43);
   size = IO.readRegister(0x43);

  switch (size)
  {
  case 49:
    size = IO.readRegister(0x43);
    if (size==49)    {
      return 11; // 1.1" detected
    } else {
      return 14; // 1.4" detected
    }        
    break;

  case 50:
    return 21;
    break;
  case 51:
    return 31;
    break;
  default:
    size = 99;
    break;
  }
  return size;
}

void PlasticLogic021::setRotation(uint8_t o) {
   IO.csStateLow();
   IO.data(EPD_DATENTRYMODE);
   switch (o)
   {
   case 2:
     /* Portrait */
     IO.data(0x02);
     break;
   
   default:
     /* Landscape mode */
     IO.data(0x20);
     break;
   }
}


void PlasticLogic021::fillScreen(uint16_t color)
{
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  uint8_t data = (color == EPD_BLACK) ? PLOGIC021_8PIX_BLACK : PLOGIC021_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

uint16_t PlasticLogic021::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  printf("_setPartialRamArea not used in PlasticLogic021");
  return 0;
}
void PlasticLogic021::_wakeUp(){
  printf("_wakeUp not used in PlasticLogic021");
}

void PlasticLogic021::_wakeUp(uint8_t em){
  printf("wakeup() start commands\n");
}

void PlasticLogic021::update(){
  update(EPD_UPD_FULL);
}

void PlasticLogic021::update(uint8_t updateMode)
{
  
  
  // EPD_PIXELACESSPOS
  printf("Sending BUFF with size: %d\n", sizeof(_buffer));
  IO.csStateLow();
  IO.data(EPD_PIXELACESSPOS);
  IO.data(0x00);
  IO.data(0x00);
  IO.csStateToogle();

  IO.data(0x10);
  // Send buffer
  for (int i=0; i < sizeof(_buffer); i++) {
    //IO.data(_buffer[i]);
  }
  
  IO.csStateHigh();
  _waitBusy("_Update_Full", EPD_TMG_SRT);
  
  _powerOn();

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
    
  /* _powerOff();
  _sleep(); */
}

void PlasticLogic021::_powerOn(void) {
  IO.csStateLow();
  IO.data(EPD_SETRESOLUTION);
  IO.data(0x00);
  IO.data(0xEF); //239
  IO.data(0x00);
  IO.data(0x9F); //159
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

  while (IO.readRegister(0x15) == 0) {}   // Wait until Internal Pump is ready 
  //_waitBusy("_PowerOn");
}

void PlasticLogic021::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  
  printf("Not implemented\n");
  vTaskDelay(PLOGIC021_PU_DELAY/portTICK_RATE_MS); 
}

void PlasticLogic021::_waitBusy(const char* message, uint16_t busy_time){
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

void PlasticLogic021::_waitBusy(const char* message){
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
void PlasticLogic021::_powerOff(){
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
void PlasticLogic021::_sleep(){
  IO.csStateLow();
  IO.data(0x21);  // deepsleep
  IO.data(0xff);          
  IO.data(0xff);
  IO.data(0xff);
  IO.data(0xff);
  IO.cs2StateHigh();
}

void PlasticLogic021::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = PLOGIC021_WIDTH - x - w - 1;
      break;
    case 2:
      x = PLOGIC021_WIDTH - x - w - 1;
      y = PLOGIC021_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = PLOGIC021_HEIGHT - y - h - 1;
      break;
  }
}


void PlasticLogic021::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = PLOGIC021_WIDTH - x - 1;
      break;
    case 2:
      x = PLOGIC021_WIDTH - x - 1;
      y = PLOGIC021_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = PLOGIC021_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * PLOGIC021_WIDTH / 8;

  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
}

    void PlasticLogic021::accelBegin() {
      uint8_t accGsel[2] = {0x0F, ACC_GSEL};
      uint8_t accBand[2] = {0x10, ACC_BW};
      IO.cmdAccel(accGsel, sizeof(accGsel));
      IO.cmdAccel(accBand, sizeof(accBand));
    }
    
    void PlasticLogic021::accelActivateTapOnInt1() {
      uint8_t accEnSingleTap[2]   = {0x16, 0x20};
      uint8_t accMapSingleTap[2]  = {0x19, 0x20};
      uint8_t accAdjTapSens[2]    = {0x2B, 0x01};
      uint8_t accInterruptMode[2] = {0x21, 0x0F};
      IO.cmdAccel(accEnSingleTap, sizeof(accEnSingleTap));
      IO.cmdAccel(accMapSingleTap, sizeof(accMapSingleTap));
      IO.cmdAccel(accAdjTapSens, sizeof(accAdjTapSens));
      IO.cmdAccel(accInterruptMode, sizeof(accInterruptMode));
    }

    void PlasticLogic021::accelClearLatchedInt1() {
      uint8_t accLatch[2] = {0x21, 0x80};
      IO.cmdAccel(accLatch, sizeof(accLatch));
    }

    void PlasticLogic021::accelReadAccel() {
      // Research how to read from SPI (MISO gpio?)
    }

    void PlasticLogic021::accelDeepSuspend() {
      uint8_t accelDeepSuspend[2] = {0x11, 0x20};
      IO.cmdAccel(accelDeepSuspend, sizeof(accelDeepSuspend));
    }
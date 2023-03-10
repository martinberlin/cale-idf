// Controller:        UC8253
// GDEQ037T31_416x240
#include "goodisplay/gdeq037T31.h"

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Constructor
Gdeq037T31::Gdeq037T31(EpdSpi& dio): 
  Adafruit_GFX(GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT),
  Epd(GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT), IO(dio)
{
  printf("Gdeq037T31() %d*%d\n",
  GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT);  
}

void Gdeq037T31::initFullUpdate(){
    _wakeUp();
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

//Initialize the display
void Gdeq037T31::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdeq037T31::init(%d)\n", debug);
    IO.init(4, debug); // 4MHz frequency

    printf("Free heap:%d\n", (int)xPortGetFreeHeapSize());
    fillScreen(EPD_WHITE);
    _mono_mode = 1;
    fillScreen(EPD_WHITE);
}

void Gdeq037T31::fillScreen(uint16_t color)
{
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  uint8_t data = (color == EPD_BLACK) ? GDEQ037T31_8PIX_BLACK : GDEQ037T31_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
  {
    _mono_buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _mono_buffer len:%d\n",color,sizeof(_mono_buffer));
}

void Gdeq037T31::_wakeUp(){
  IO.reset(10);
  IO.cmd(0x04);
  _waitBusy("_PowerOn");

  IO.cmd(0x50);
  IO.data(0x97);
  // Marked in GOODISPLAY as EPD_init_Fast
  if (fast_mode) {
    IO.cmd(0xE0);
    IO.data(0x02);
    IO.cmd(0xE5);
    IO.data(0x5A);
  }
}

void Gdeq037T31::update()
{
  uint64_t startTime = esp_timer_get_time();
  uint8_t xLineBytes = GDEQ037T31_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  _wakeUp();

  uint64_t endTime = esp_timer_get_time();
  if (total_updates) {
    // Old buffer update so the display can compare
    IO.cmd(0x10);
    for (uint16_t y = GDEQ037T31_HEIGHT; y > 0; y--)
      {
        for (uint16_t x = 0; x < xLineBytes; x++)
        {
          uint16_t idx = y * xLineBytes + x;  
          x1buf[x] = ~_old_buffer[idx];
        }
        IO.data(x1buf, sizeof(x1buf));
      }
  }

  IO.cmd(0x13);
  for (uint16_t y = GDEQ037T31_HEIGHT; y > 0; y--)
    {
      for (uint16_t x = 0; x < xLineBytes; x++)
      {
        uint16_t idx = y * xLineBytes + x;  
        x1buf[x] = ~_mono_buffer[idx];
      }
      IO.data(x1buf, sizeof(x1buf));
    }

  IO.cmd(0x12);
  _waitBusy("_Update_Full");
  uint64_t powerOnTime = esp_timer_get_time();
  total_updates++;
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  memcpy(_old_buffer, _mono_buffer, GDEQ037T31_BUFFER_SIZE);
  _sleep();
}

void Gdeq037T31::_setRamDataEntryMode(uint8_t em)
{
  const uint16_t xPixelsPar = GDEQ037T31_WIDTH - 1;
  const uint16_t yPixelsPar = GDEQ037T31_HEIGHT - 1;
  em = gx_uint16_min(em, 0x03);
  IO.cmd(0x11);
  IO.data(em);
  switch (em)
  {
    case 0x00: // x decrease, y decrease
      _SetRamArea(xPixelsPar / 8, 0x00, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x01: // x increase, y decrease : as in demo code
      _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x02: // x decrease, y increase
      _SetRamArea(xPixelsPar / 8, 0x00, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, 0x00, 0x00); // set ram
      break;
    case 0x03: // x increase, y increase : normal mode
      _SetRamArea(0x00, xPixelsPar / 8, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(0x00, 0x00, 0x00); // set ram
      break;
  }
}

void Gdeq037T31::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  IO.cmd(0x44);
  IO.data(Xstart);
  IO.data(Xend);
  IO.cmd(0x45);
  IO.data(Ystart);
  IO.data(Ystart1);
  IO.data(Yend);
  IO.data(Yend1);
}

void Gdeq037T31::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  IO.cmd(0x4e);
  IO.data(addrX);
  IO.cmd(0x4f);
  IO.data(addrY);
  IO.data(addrY1);
}

void Gdeq037T31::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (!_using_partial_mode) {
    _using_partial_mode = true;
    _wakeUp();

    // Fix gray partial update
    IO.cmd(0x26);
    for (int16_t i = 0; i <= GDEQ037T31_BUFFER_SIZE; i++)
    {
      IO.data(0xFF);
    }
  }
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEQ037T31_WIDTH) return;
  if (y >= GDEQ037T31_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEQ037T31_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEQ037T31_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8;

  IO.cmd(0x12); //SWRESET
  _waitBusy("SWRESET");
  _setRamDataEntryMode(0x03);
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  //_waitBusy("partialUpdate1", 100); // needed ?

  IO.cmd(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEQ037T31_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_mono_buffer)) ? _mono_buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  

  IO.cmd(0x22);
  IO.data(0xFF); //0x04
  IO.cmd(0x20);
  _waitBusy("updateWindow");
}

void Gdeq037T31::_waitBusy(const char* message){
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

void Gdeq037T31::_sleep(){
  IO.cmd(0x22); // power off display
  IO.data(0xc3);
  IO.cmd(0x20);
  _waitBusy("power_off");
}

void Gdeq037T31::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEQ037T31_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEQ037T31_WIDTH - x - w - 1;
      y = GDEQ037T31_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEQ037T31_HEIGHT - y - h - 1;
      break;
  }
}


void Gdeq037T31::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEQ037T31_WIDTH - x - 1;
      break;
    case 2:
      x = GDEQ037T31_WIDTH - x - 1;
      y = GDEQ037T31_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEQ037T31_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEQ037T31_WIDTH / 8;

  if (color) {
    _mono_buffer[i] = (_mono_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _mono_buffer[i] = (_mono_buffer[i] | (1 << (7 - x % 8)));
    }
}

void Gdeq037T31::setMonoMode(bool mode) {
  ESP_LOGI("Gdeq037T31", "No known 4 gray mode in this display");
}

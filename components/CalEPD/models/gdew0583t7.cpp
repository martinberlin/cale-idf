#include "gdew0583t7.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// CMD, DATA, Databytes * Optional we are going to use sizeof(data)
DRAM_ATTR const epd_init_2 Gdew0583T7::epd_wakeup_power={
0x01,{0x37,0x00},2
};

DRAM_ATTR const epd_init_2 Gdew0583T7::epd_panel_setting={
0x00,{0xCF,0x08},2
};

DRAM_ATTR const epd_init_3 Gdew0583T7::epd_boost={
0x06,{0xC7,0xCC,0x28},3
};

// 0x3a -> 15s refresh  |  0x3c -> 30s refresh
DRAM_ATTR const epd_init_1 Gdew0583T7::epd_pll={
0x30,{0x3a},1
};

DRAM_ATTR const epd_init_1 Gdew0583T7::epd_temperature={
0x41,{0x00},1
};

DRAM_ATTR const epd_init_4 Gdew0583T7::epd_resolution={
0x61,{
  0x02, //source 600
  0x58,
  0x01, //gate 448
  0xc0
},4};

// Constructor
Gdew0583T7::Gdew0583T7(EpdSpi& dio): 
  Adafruit_GFX(GDEW0583T7_WIDTH, GDEW0583T7_HEIGHT),
  Epd(GDEW0583T7_WIDTH, GDEW0583T7_HEIGHT), IO(dio)
{
  printf("Gdew0583T7() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0583T7_WIDTH, GDEW0583T7_HEIGHT);  
}

//Initialize the display
void Gdew0583T7::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0583T7::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug
    IO.init(4, debug);
    fillScreen(EPD_WHITE);
}

void Gdew0583T7::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Gdew0583T7::_wakeUp(){
  IO.reset(10);
//IMPORTANT: Some EPD controllers like to receive data byte per byte
//So this won't work, still needs to be tried out for this epaper:
//IO.data(epd_wakeup_power.data,epd_wakeup_power.databytes);
  printf("_wakeUp Power on\n");
  IO.cmd(epd_wakeup_power.cmd);
  for (int i=0;i<sizeof(epd_wakeup_power.data);++i) {
    //printf(">%d\n",i);
    IO.data(epd_wakeup_power.data[i]);
  }
 
  printf("Panel setting\n");
  IO.cmd(epd_panel_setting.cmd);
  for (int i=0;i<sizeof(epd_panel_setting.data);++i) {
    IO.data(epd_panel_setting.data[i]);
  }

  IO.cmd(epd_boost.cmd);
  for (int i=0;i<sizeof(epd_boost.data);++i) {
    IO.data(epd_boost.data[i]);
  }

  IO.cmd(epd_pll.cmd);
  IO.data(epd_pll.data[0]);

  IO.cmd(epd_temperature.cmd);
  IO.data(epd_temperature.data[0]);
  // Vcom and data interval settings
  IO.cmd(0x50);
  IO.data(0x77);
  IO.cmd(0x60); // TCON (???)
  IO.data(0x22);

  // Resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<sizeof(epd_resolution.data);++i) {
    IO.data(epd_resolution.data[i]);
  }

  // Vcom voltage setting
  IO.cmd(0x82);  // VCOM Voltage
  IO.data(0x28); // All temperature range
  
  IO.cmd(0xe5);  // Flash mode
  IO.data(0x03);

  // Power it on
  IO.cmd(0x04);
  _waitBusy("Power on");
}

void Gdew0583T7::_send8pixel(uint8_t data)
{
  for (uint8_t j = 0; j < 8; j++)
  {
    uint8_t t = data & 0x80 ? 0x00 : 0x03;
    t <<= 4;
    data <<= 1;
    j++;
    t |= data & 0x80 ? 0x00 : 0x03;
    data <<= 1;
    IO.dataBuffer(t);
  }
}

void Gdew0583T7::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n",sizeof(_buffer));

  for (uint32_t i = 0; i < sizeof(_buffer); i++)
  {
    // If this does not work please comment this:
    _send8pixel(i < sizeof(_buffer) ? _buffer[i] : 0x00);

    if (i%2000==0) {
       #if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
       rtc_wdt_feed();
       #endif
       vTaskDelay(pdMS_TO_TICKS(10));
       if (debug_enabled) printf("%d ", (int) i);
    }
  
  }
  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);

  //vTaskDelay(pdMS_TO_TICKS(1000));
  _sleep();
}

uint16_t Gdew0583T7::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8;            // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.cmd(0x90);           // partial window
  IO.data(x / 256);
  IO.data(x % 256);
  IO.data(xe / 256);
  IO.data(xe % 256);
  IO.data(y / 256);
  IO.data(y % 256);
  IO.data(ye / 256);
  IO.data(ye % 256);
  IO.data(0x00);
  return (7 + xe - x) / 8; // number of bytes to transfer per line

}

void Gdew0583T7::eraseDisplay(bool using_partial_update) {
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true;   // remember
    IO.cmd(0x91);                 // partial in
    _setPartialRamArea(0, 0, GDEW0583T7_WIDTH - 1, GDEW0583T7_HEIGHT - 1);
    IO.cmd(0x10);
    for (uint32_t i = 0; i < GDEW0583T7_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.cmd(0x12);                 // display refresh
    _waitBusy("eraseDisplay");
    IO.cmd(0x92);                 // partial out
  } else {
    // Not using Partial update
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.cmd(0x10);
    for (uint32_t i = 0; i < GDEW0583T7_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.cmd(0x12);      //display refresh
    _waitBusy("eraseDisplay");
    _sleep();
  }
}

void Gdew0583T7::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Still in test mode\n");
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEW0583T7_WIDTH) return;
  if (y >= GDEW0583T7_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEW0583T7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEW0583T7_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) eraseDisplay(true); // clean surrounding
  _using_partial_mode = true;
   
  IO.cmd(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.cmd(0x10);

  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GDEW0583T7_WIDTH / 8) + x1;
      _send8pixel((idx < sizeof(_buffer)) ? _buffer[idx] : 0x00);
    }
  }
  IO.cmd(0x12);     // display refresh
  _waitBusy("updateWindow partial refresh");
  IO.cmd(0x92);     // partial out
  
  vTaskDelay(GDEW0583T7_PU_DELAY / portTICK_PERIOD_MS);
}

void Gdew0583T7::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // In this controller BUSY == 0 
  while (true){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew0583T7::_sleep(){
  IO.cmd(0X65); // Flash control (???)
  IO.data(0x01);
  IO.cmd(0xB9);
  IO.cmd(0X65); // Flash control
  IO.data(0x00);

  // Flash sleep  
  IO.cmd(0x02);
  _waitBusy("Power Off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xa5);
}

void Gdew0583T7::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0583T7_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0583T7_WIDTH - x - w - 1;
      y = GDEW0583T7_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0583T7_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0583T7::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0583T7_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0583T7_WIDTH - x - 1;
      y = GDEW0583T7_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0583T7_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0583T7_WIDTH / 8;

  if (!color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
}

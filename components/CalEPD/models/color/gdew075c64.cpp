#include "color/gdew075c64.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Controller: GD7965 (EK79655)
// Specification: https://www.scribd.com/document/448888338/GDEW075C64-V1-0-Specification

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V
DRAM_ATTR const epd_power_4 Gdew075C64::epd_wakeup_power = {
    0x01, {0x07, 0x07, 0x3f, 0x3f}, 4};

DRAM_ATTR const epd_init_4 Gdew075C64::epd_resolution = {
    0x61, {0x03, //source 800
           0x20,
           0x01, //gate 480
           0xE0},
    4};

// Note: Played with this settings. If is too high (In specs says 0x17) then
//       yellow will get out from right side. Too low and won't be yellow enough (But is sill not 100% right)
//       For me it looks more yellow on the top and more dark yellow on the bottom
DRAM_ATTR const epd_init_4 Gdew075C64::epd_boost={
0x06,{
  0x17,
  0x17,
  0x25, /* Top part of the display get's more color on 0x16. On 0x18 get's too yellow and desbords on left side. On 0x17 is shit */
  0x17},4
};

// Constructor
Gdew075C64::Gdew075C64(EpdSpi &dio) : Adafruit_GFX(GDEW075C64_WIDTH, GDEW075C64_HEIGHT),
                                    Epd(GDEW075C64_WIDTH, GDEW075C64_HEIGHT), IO(dio)
{
  printf("Gdew075C64() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075C64_WIDTH, GDEW075C64_HEIGHT, (int)GDEW075C64_BUFFER_SIZE);
}

//Initialize the display
void Gdew075C64::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdew075C64::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
}

void Gdew075C64::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == EPD_WHITE);
  else if (color == EPD_BLACK) black = 0xFF;
  else if (color == EPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = black;
    _color[x] = red;
  }
}

void Gdew075C64::_wakeUp()
{
  IO.reset(10);
  //IMPORTANT: Some EPD controllers like to receive data byte per byte
  //So this won't work:
  //IO.data(epd_wakeup_power.data,epd_wakeup_power.databytes);

  IO.cmd(epd_wakeup_power.cmd);
  for (int i = 0; i < epd_wakeup_power.databytes; ++i)
  {
    IO.data(epd_wakeup_power.data[i]);
  }

  IO.cmd(0x04);
  _waitBusy("_wakeUp power on");

  // Panel setting
  IO.cmd(0x00);
  IO.data(0x0f); //KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f

  // PLL 
  IO.cmd(0x30);
  IO.data(0x06);

  // Resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i = 0; i < epd_resolution.databytes; ++i)
  {
    IO.data(epd_resolution.data[i]);
  }

  printf("Boost\n"); // Handles the intensity of the colors displayed
  IO.cmd(epd_boost.cmd);
  for (int i=0;i<sizeof(epd_boost.data);++i) {
    IO.data(epd_boost.data[i]);
  }

  // Not sure if 0x15 is really needed, seems to work the same without it too
  IO.cmd(0x15);  // Dual SPI
  IO.data(0x00); // MM_EN, DUSPI_EN

  IO.cmd(0x50);  // VCOM AND DATA INTERVAL SETTING
  IO.data(0x11); // LUTKW, N2OCP: copy new to old
  IO.data(0x07);

  IO.cmd(0x60);  // TCON SETTING
  IO.data(0x22);
}

void Gdew075C64::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

   // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW075C64_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEW075C64_HEIGHT; y++)
  {
    for (uint16_t x = 1; x <= xLineBytes; x++)
    {
      uint8_t data = i < sizeof(_buffer) ? ~_buffer[i] : 0x00;
      x1buf[x - 1] = data;
      if (x == xLineBytes)
      { // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }
      ++i;
    }
  }

  IO.cmd(0x13);
  i = 0;
  for (uint16_t y = 1; y <= GDEW075C64_HEIGHT; y++)
  {
    for (uint16_t x = 1; x <= xLineBytes; x++)
    {
      uint8_t data = i < sizeof(_color) ? _color[i] : 0x00;
      x1buf[x - 1] = data;
      if (x == xLineBytes)
      { // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }
      ++i;
    }
  }

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);

  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();

  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);

  _sleep();
}

void Gdew075C64::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Not for color epapers of Goodisplay. Launching full update\n");
   update();
}

void Gdew075C64::_waitBusy(const char *message)
{
  if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1)
  {
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1)
      break;
    vTaskDelay(1);
    if (esp_timer_get_time() - time_since_boot > 2000000)
    {
      if (debug_enabled)
        ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew075C64::_sleep()
{
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xA5);
}

void Gdew075C64::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEW075C64_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEW075C64_WIDTH - x - w - 1;
    y = GDEW075C64_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEW075C64_HEIGHT - y - h - 1;
    break;
  }
}

void Gdew075C64::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEW075C64_WIDTH - x - 1;
    break;
  case 2:
    x = GDEW075C64_WIDTH - x - 1;
    y = GDEW075C64_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEW075C64_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEW075C64_WIDTH / 8;

  _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _color[i] = (_color[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else if (color == EPD_RED) _color[i] = (_color[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _color[i] = (_color[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}

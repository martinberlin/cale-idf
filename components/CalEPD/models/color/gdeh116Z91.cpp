#include "gdeh116Z91.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

DRAM_ATTR const epd_init_5 Gdeh116Z91::soft_start = {
    0x0C, {0xAE, 0xC7, 0xC3, 0xC0, 0x40}, 5};

// Constructor
Gdeh116Z91::Gdeh116Z91(EpdSpi &dio) : Adafruit_GFX(GDEH116Z91_WIDTH, GDEH116Z91_HEIGHT),
                                    Epd(GDEH116Z91_WIDTH, GDEH116Z91_HEIGHT), IO(dio)
{
  printf("Gdeh116Z91() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEH116Z91_WIDTH, GDEH116Z91_HEIGHT, GDEH116Z91_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

//Initialize the display
void Gdeh116Z91::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdeh116Z91::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
}

void Gdeh116Z91::fillScreen(uint16_t color)
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

void Gdeh116Z91::_wakeUp()
{
  //IO.reset(10);
  IO.cmd(0x12); //SWRESET
  _waitBusy("_wakeUp Power on");

  IO.cmd(soft_start.cmd);
  for (int i = 0; i < soft_start.databytes; ++i)
  {
    IO.data(soft_start.data[i]);
  }

  IO.cmd(0x01);  // Set MUX as 639
  IO.data(0x7f);
  IO.data(0x02);
  IO.data(0x00);

  // Partial RAM area
  IO.cmd(0x44);
  IO.data(0x00); // RAM x address start at 0
  IO.data(0x00);
  IO.data(0xBF); // RAM x address end at 3BFh -> 959
  IO.data(0x03);
  IO.cmd(0x45);
  IO.data(0x7F); // RAM y address start at 27Fh;
  IO.data(0x02);
  IO.data(0x00); // RAM y address end at 00h;
  IO.data(0x00);

  IO.cmd(0x3C); // VBD
  IO.data(0x01);    // LUT1, for white
  IO.cmd(0x18); // Temperature Sensor Selection
  IO.data(0x80);    // internal temperature sensor
  // Power on
  IO.cmd(0x22); // Display Update Sequence Options
  IO.data(0xB1);    // Load Temperature and waveform setting.
  IO.cmd(0x20); // Master Activation
}

void Gdeh116Z91::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x24);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

   // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEH116Z91_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEH116Z91_HEIGHT; y++)
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

  IO.cmd(0x26);
  i = 0;
  for (uint16_t y = 1; y <= GDEH116Z91_HEIGHT; y++)
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


void Gdeh116Z91::_waitBusy(const char *message)
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

void Gdeh116Z91::_sleep()
{
  printf("Deepsleep\n");
  IO.cmd(0x10); // Deep sleep
  IO.data(0x11);
}

void Gdeh116Z91::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEH116Z91_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEH116Z91_WIDTH - x - w - 1;
    y = GDEH116Z91_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEH116Z91_HEIGHT - y - h - 1;
    break;
  }
}

void Gdeh116Z91::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEH116Z91_WIDTH - x - 1;
    break;
  case 2:
    x = GDEH116Z91_WIDTH - x - 1;
    y = GDEH116Z91_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEH116Z91_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEH116Z91_WIDTH / 8;

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

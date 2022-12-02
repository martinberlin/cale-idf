#include "gdew075HD.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Constructor
Gdew075HD::Gdew075HD(EpdSpi &dio) : Adafruit_GFX(GDEW075HD_WIDTH, GDEW075HD_HEIGHT),
                                    Epd(GDEW075HD_WIDTH, GDEW075HD_HEIGHT), IO(dio)
{
  printf("Gdew075HD() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075HD_WIDTH, GDEW075HD_HEIGHT, (int)GDEW075HD_BUFFER_SIZE);
}

//Initialize the display
void Gdew075HD::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdew075HD::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
}

void Gdew075HD::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? GDEW075HD_8PIX_BLACK : GDEW075HD_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Gdew075HD::_wakeUp()
{
    IO.reset(200);
    
    IO.cmd(0x12);

    _waitBusy("Reset_12");
  
    IO.cmd(0x46);  // Auto Write Red RAM
    IO.data(0xf7);
    _waitBusy("Write Red RAM");

    IO.cmd(0x47);  // Auto Write  B/W RAM
    IO.data(0xf7);
    _waitBusy("Write B/W RAM");

    IO.cmd(0x0C);  // Soft start setting
    IO.data(0xAE);
    IO.data(0xC7);
    IO.data(0xC3);
    IO.data(0xC0);
    IO.data(0x40); 

    IO.cmd(0x01);  // Set MUX as 527
    IO.data(0xAF);
    IO.data(0x02);
    IO.data(0x01); //0x01

    IO.cmd(0x11);  // Data entry mode      
    IO.data(0x01);

    IO.cmd(0x44); 
    IO.data(0x00); // RAM x address start at 0
    IO.data(0x00); 
    IO.data(0x6F); 
    IO.data(0x03); 

    IO.cmd(0x45); 
    IO.data(0xAF); 
    IO.data(0x02);
    IO.data(0x00); 
    IO.data(0x00);

    IO.cmd(0x3C);  // VBD
    IO.data(0x01); // LUT1, for white

    IO.cmd(0x18); // Temperature Sensor Control
    IO.data(0X80);

    IO.cmd(0x22);
    IO.data(0XB1); //Load Temperature and waveform setting.
    IO.cmd(0x20);
    _waitBusy("Load Temperature");
    
    IO.cmd(0x4E); // set RAM x address count to 0;
    IO.data(0x00);
    IO.data(0x00);

    IO.cmd(0x4F); 
    IO.data(0x00);
    IO.data(0x00);

    IO.cmd(0x4F); 
    IO.data(0x00);
    IO.data(0x00);


    IO.cmd(0x24);//BLOCK
}

void Gdew075HD::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x24); //Black RAM
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW075HD_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEW075HD_HEIGHT; y++)
  {
    for (uint16_t x = 1; x <= xLineBytes; x++)
    {
      uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x00;
      x1buf[x - 1] = data;
      if (x == xLineBytes)
      { // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }
      ++i;
    }
  } 

  /* 
  for (uint16_t i = 1; i <= GDEW075HD_BUFFER_SIZE; i++)
  {
    IO.data(_buffer[i]);
    if (i<50) {
      printf("%x ", _buffer[i]);
    }
  } */

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x22);  // Show
  IO.data(0xF7); // 0xF7
  IO.cmd(0x20);

  vTaskDelay(200 / portTICK_PERIOD_MS);
  _waitBusy("Update");

  uint64_t updateTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);
  
  // Additional 2 seconds wait before sleeping since in low temperatures full update takes longer
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  _sleep();
}

void Gdew075HD::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Not implemented\n");
}

void Gdew075HD::_waitBusy(const char *message)
{
  if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1)
  {
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) {
      break;
      }
      vTaskDelay(1);
    if (esp_timer_get_time() - time_since_boot > 2000000)
    {
      if (debug_enabled)
        ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew075HD::_sleep()
{
  IO.cmd(0x10);
  IO.data(0x01);
}

void Gdew075HD::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEW075HD_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEW075HD_WIDTH - x - w - 1;
    y = GDEW075HD_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEW075HD_HEIGHT - y - h - 1;
    break;
  }
}

void Gdew075HD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEW075HD_WIDTH - x - 1;
    break;
  case 2:
    x = GDEW075HD_WIDTH - x - 1;
    y = GDEW075HD_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEW075HD_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEW075HD_WIDTH / 8;

  if (color)
  {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  }
  else
  {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  }
}

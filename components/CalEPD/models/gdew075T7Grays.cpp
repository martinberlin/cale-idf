#include "gdew075T7Grays.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Partial Update Delay, may have an influence on degradation
#define GDEW075T7_PU_DELAY 100


// Grays Waveform
DRAM_ATTR const epd_init_42 Gdew075T7::lut_vcom = {
    0x20, {
      0x00	,0x0A	,0x00	,0x00	,0x00	,0x01,
0x60	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x00	,0x00	,0x00	,0x01,
0x00	,0x13	,0x0A	,0x01	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00}, 42};

DRAM_ATTR const epd_init_42 Gdew075T7::lut_ww = {
    0x21, {
           0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x10	,0x14	,0x0A	,0x00	,0x00	,0x01,
0xA0	,0x13	,0x01	,0x00	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T7::lut_bw = {
    0x22, {
           0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x99	,0x0C	,0x01	,0x03	,0x04	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T7::lut_wb = {
    0x23, {
          0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x99	,0x0B	,0x04	,0x04	,0x01	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00
          },
    42};

DRAM_ATTR const epd_init_42 Gdew075T7::lut_bb = {
    0x24, {//R24H	b
           0x80	,0x0A	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x20	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x50	,0x13	,0x01	,0x00	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V
DRAM_ATTR const epd_power_4 Gdew075T7::epd_wakeup_power = {
    0x01, {0x07, 0x17, 0x3f, 0x3f}, 4};

DRAM_ATTR const epd_init_1 Gdew075T7::epd_panel_setting_full = {
    0x00, {0x1f}, 1};

DRAM_ATTR const epd_init_1 Gdew075T7::epd_panel_setting_partial = {
    0x00, {0x3f}, 1};

DRAM_ATTR const epd_init_4 Gdew075T7::epd_resolution = {
    0x61, {GDEW075T7_WIDTH / 256, //source 800
           GDEW075T7_WIDTH % 256,
           GDEW075T7_HEIGHT / 256, //gate 480
           GDEW075T7_HEIGHT % 256},
    4};

// Constructor
Gdew075T7::Gdew075T7(EpdSpi &dio) : Adafruit_GFX(GDEW075T7_WIDTH, GDEW075T7_HEIGHT),
                                    Epd(GDEW075T7_WIDTH, GDEW075T7_HEIGHT), IO(dio)
{
  printf("Gdew075T7() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075T7_WIDTH, GDEW075T7_HEIGHT, GDEW075T7_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

void Gdew075T7::initFullUpdate()
{
  IO.cmd(epd_panel_setting_full.cmd);      // panel setting
  IO.data(epd_panel_setting_full.data[0]); // full update LUT from OTP
// LUT for grays
  IO.cmd(0x20);
  IO.data(lut_vcom.data, 42);
  IO.cmd(0x21);
  IO.data(lut_ww.data, 42);
  IO.cmd(0x22);
  IO.data(lut_bw.data, 42);
  IO.cmd(0x23);
  IO.data(lut_wb.data, 42);
  IO.cmd(0x24);
  IO.data(lut_bb.data, 42);
  IO.cmd(0x25);
  IO.data(lut_ww.data, 42);
}


//Initialize the display
void Gdew075T7::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdew075T7::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
}

void Gdew075T7::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? GDEW075T7_8PIX_BLACK : GDEW075T7_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Gdew075T7::_wakeUp()
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

  // Resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i = 0; i < epd_resolution.databytes; ++i)
  {
    IO.data(epd_resolution.data[i]);
  }
  IO.cmd(0x15);  // Dual SPI
  IO.data(0x00); // MM_EN, DUSPI_EN

  IO.cmd(0x60);  // TCON SETTING
  IO.data(0x22);

  IO.cmd(0x80);  //vcom_DC setting
  IO.data(0x12);

  IO.cmd(0x50);  //VCOM AND DATA INTERVAL SETTING
  IO.data(0x10); //10:KW(0--1)  21:KW(1--0)
  IO.data(0x07);
  initFullUpdate();
}

void Gdew075T7::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x13);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW075T7_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEW075T7_HEIGHT; y++)
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

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);
  
  // Additional 2 seconds wait before sleeping since in low temperatures full update takes longer
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  _sleep();
}

void Gdew075T7::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: There is no partial update using the Gdew075T7Grays class\n");
}

void Gdew075T7::_waitBusy(const char *message)
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

void Gdew075T7::_sleep()
{
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xA5);
}

void Gdew075T7::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEW075T7_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEW075T7_WIDTH - x - w - 1;
    y = GDEW075T7_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEW075T7_HEIGHT - y - h - 1;
    break;
  }
}

void Gdew075T7::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEW075T7_WIDTH - x - 1;
    break;
  case 2:
    x = GDEW075T7_WIDTH - x - 1;
    y = GDEW075T7_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEW075T7_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEW075T7_WIDTH / 8;

  if (color)
  {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  }
  else
  {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  }
}

void Gdew075T7::fillRawBufferPos(uint16_t index, uint8_t value) {
  _buffer[index] = value;
}

void Gdew075T7::fillRawBufferImage(uint8_t image[], uint16_t size) {
  for (int i=0; i<size; ++i) {
      _buffer[i] = image[i];
   }
}
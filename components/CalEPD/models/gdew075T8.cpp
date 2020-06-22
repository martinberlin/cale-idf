#include "gdew075T8.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Partial Update Delay, may have an influence on degradation
#define GDEW075T8_PU_DELAY 100
/* 
//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.

DRAM_ATTR const epd_init_42 Gdew075T8::lut_20_LUTC_partial = {
    0x20, {0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 42};

DRAM_ATTR const epd_init_42 Gdew075T8::lut_21_LUTWW_partial = {
    0x21, {// 10 w
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T8::lut_22_LUTKW_partial = {
    0x22, {// 10 w
           //0x48, T1, T2, T3, T4, 1, // 01 00 10 00
           0x5A, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T8::lut_23_LUTWK_partial = {
    0x23, {
              // 01 b
              0x84, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
              //0xA5, T1, T2, T3, T4, 1, // 10 10 01 01 more black
          },
    42};

DRAM_ATTR const epd_init_42 Gdew075T8::lut_24_LUTKK_partial = {
    0x24, {// 01 b
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T8::lut_25_LUTBD_partial = {
    0x25, {// 01 b
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V
DRAM_ATTR const epd_power_4 Gdew075T8::epd_wakeup_power = {
    0x01, {0x07, 0x07, 0x3f, 0x3f}, 4};

DRAM_ATTR const epd_init_1 Gdew075T8::epd_panel_setting_full = {
    0x00, {0x1f}, 1};

DRAM_ATTR const epd_init_1 Gdew075T8::epd_panel_setting_partial = {
    0x00, {0x3f}, 1};

DRAM_ATTR const epd_init_4 Gdew075T8::epd_resolution = {
    0x61, {GDEW075T7_WIDTH / 256, //source 800
           GDEW075T7_WIDTH % 256,
           GDEW075T7_HEIGHT / 256, //gate 480
           GDEW075T7_HEIGHT % 256},
    4};
 */
// Constructor
Gdew075T8::Gdew075T8(EpdSpi &dio) : Adafruit_GFX(GDEW075T8_WIDTH, GDEW075T8_HEIGHT),
                                    Epd(GDEW075T8_WIDTH, GDEW075T8_HEIGHT), IO(dio)
{
  printf("Gdew075T8() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075T8_WIDTH, GDEW075T8_HEIGHT, GDEW075T8_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

//Initialize the display
void Gdew075T8::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdew075T8::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, true);
  fillScreen(EPD_WHITE);
}

void Gdew075T8::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_WHITE) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Gdew075T8::_wakeUp()
{
  IO.reset(10);

  IO.cmd(0X65);     //FLASH CONTROL
  IO.data(0x01);

  IO.cmd(0xAB);

  IO.cmd(0X65);     //FLASH CONTROL
  IO.data(0x00);

  IO.cmd(0x01);
  IO.data(0x37);    //POWER SETTING
  IO.data(0x00);

  IO.cmd(0X00);     //PANNEL SETTING
  IO.data(0xCF);
  IO.data(0x08);

  IO.cmd(0x06);     //boost
  IO.data(0xc7);
  IO.data(0xcc);
  IO.data(0x28);

  IO.cmd(0x30);     //PLL setting
  IO.data(0x3c);

  IO.cmd(0X41);     //TEMPERATURE SETTING
  IO.data(0x00);

  IO.cmd(0X50);     //VCOM AND DATA INTERVAL SETTING
  IO.data(0x77);

  IO.cmd(0X60);     //TCON SETTING
  IO.data(0x22);

  IO.cmd(0x61);     //tres 640*384
  IO.data(0x02);    //source 640
  IO.data(0x80);
  IO.data(0x01);    //gate 384
  IO.data(0x80);

  IO.cmd(0X82);     //VDCS SETTING
  IO.data(0x1E);    //decide by LUT file

  IO.cmd(0xe5);     //FLASH MODE
  IO.data(0x03);

  IO.cmd(0x04);     //POWER ON
  _waitBusy("_wakeUp power on");

}

void Gdew075T8::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW075T8_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEW075T8_HEIGHT; y++)
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

  _sleep();
}

uint16_t Gdew075T8::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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

void Gdew075T8::_send8pixel(uint8_t data)
{
  for (uint8_t j = 0; j < 8; j++)
  {
    uint8_t t = data & 0x80 ? 0x00 : 0x03;
    t <<= 4;
    data <<= 1;
    j++;
    t |= data & 0x80 ? 0x00 : 0x03;
    data <<= 1;
    IO.data(t);
  }
}

void Gdew075T8::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Still in test mode\n");
  
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(x, y);
        swap(w, h);
        x = GDEW075T8_WIDTH - x - w - 1;
        break;
      case 2:
        x = GDEW075T8_WIDTH - x - w - 1;
        y = GDEW075T8_HEIGHT - y - h - 1;
        break;
      case 3:
        swap(x, y);
        swap(w, h);
        y = GDEW075T8_HEIGHT - y  - h - 1;
        break;
    }
  }
  if (x >= GDEW075T8_WIDTH) return;
  if (y >= GDEW075T8_HEIGHT) return;
  // x &= 0xFFF8; // byte boundary, not here, use encompassing rectangle
  uint16_t xe = gx_uint16_min(GDEW075T8_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEW075T8_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  //if (!_using_partial_mode) _wakeUp();
  if (!_using_partial_mode) eraseDisplay(true); // clean surrounding
  _using_partial_mode = true;
  IO.cmd(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.cmd(0x10);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GDEW075T8_WIDTH / 8) + x1;
      _send8pixel((idx < sizeof(_buffer)) ? _buffer[idx] : 0x00);
    }
  }
  IO.cmd(0x12);      //display refresh
  _waitBusy("updateWindow");
  IO.cmd(0x92); // partial out

  vTaskDelay(GDEW075T8_PU_DELAY / portTICK_PERIOD_MS);
}

void Gdew075T8::eraseDisplay(bool using_partial_update)
{
  if (using_partial_update)
  {
    if (!_using_partial_mode) _wakeUp();
    _using_partial_mode = true; // remember
    IO.cmd(0x91);               // partial in
    _setPartialRamArea(0, 0, GDEW075T8_WIDTH - 1, GDEW075T8_HEIGHT - 1);
    IO.cmd(0x10);
    for (uint32_t i = 0; i < GDEW075T8_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.cmd(0x12);                //display refresh
    _waitBusy("eraseDisplay");
    IO.cmd(0x92);                // partial out
  }
  else
  {
    _using_partial_mode = false; // remember
    _wakeUp();
    IO.cmd(0x10);
    for (uint32_t i = 0; i < GDEW075T8_BUFFER_SIZE; i++)
    {
      _send8pixel(0x00);
    }
    IO.cmd(0x12);                //display refresh
    _waitBusy("eraseDisplay");
    _sleep();
  }
}

void Gdew075T8::_waitBusy(const char *message)
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

void Gdew075T8::_sleep()
{
  IO.cmd(0X65);     //FLASH CONTROL
  IO.data(0x01);

  IO.cmd(0xB9);
  IO.cmd(0X65);     //FLASH CONTROL
  IO.data(0x00);
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07);     // Deep sleep
  IO.data(0xA5);
}

void Gdew075T8::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEW075T8_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEW075T8_WIDTH - x - w - 1;
    y = GDEW075T8_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEW075T8_HEIGHT - y - h - 1;
    break;
  }
}

void Gdew075T8::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEW075T8_WIDTH - x - 1;
    break;
  case 2:
    x = GDEW075T8_WIDTH - x - 1;
    y = GDEW075T8_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEW075T8_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEW075T8_WIDTH / 8;

  if (color)
  {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  }
  else
  {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  }
}

#include "goodisplay/gdey075T7.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
/*
 The EPD needs a bunch of command/data values to be initialized. They are send using the IO class
 Manufacturer sample: https://github.com/waveshare/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp
*/
#define T1 0x19 // charge balance pre-phase
#define T2 0x01  // optional extension
#define T3 0x00 // color change phase (b/w)
#define T4 0x00  // optional extension for one color

// Partial Update Delay, may have an influence on degradation
#define GDEY075T7_PU_DELAY 100

// Partial display Waveform
DRAM_ATTR const epd_init_42 Gdey075T7::lut_20_LUTC_partial = {
    0x20, {0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 42};

DRAM_ATTR const epd_init_42 Gdey075T7::lut_21_LUTWW_partial = {
    0x21, {
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdey075T7::lut_22_LUTKW_partial = {
    0x22, {
           0x80, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdey075T7::lut_23_LUTWK_partial = {
    0x23, {
          0x40, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
          //0xA5 more black
          },
    42};

DRAM_ATTR const epd_init_42 Gdey075T7::lut_24_LUTKK_partial = {
    0x24, {// 01 b
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

DRAM_ATTR const epd_init_42 Gdey075T7::lut_25_LUTBD_partial = {
    0x25, {// 01 b
           0x00, T1, T2, T3, T4, 1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    42};

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V
DRAM_ATTR const epd_power_4 Gdey075T7::epd_wakeup_power = {
    0x01, {0x07, 0x07, 0x3f, 0x3f}, 4};

DRAM_ATTR const epd_init_1 Gdey075T7::epd_panel_setting_full = {
    0x00, {0x1f}, 1};

DRAM_ATTR const epd_init_1 Gdey075T7::epd_panel_setting_partial = {
    0x00, {0x3f}, 1};

DRAM_ATTR const epd_init_4 Gdey075T7::epd_resolution = {
    0x61, {GDEY075T7_WIDTH / 256, //source 800
           GDEY075T7_WIDTH % 256,
           GDEY075T7_HEIGHT / 256, //gate 480
           GDEY075T7_HEIGHT % 256},
    4};

// Constructor
Gdey075T7::Gdey075T7(EpdSpi &dio) : Adafruit_GFX(GDEY075T7_WIDTH, GDEY075T7_HEIGHT),
                                    Epd(GDEY075T7_WIDTH, GDEY075T7_HEIGHT), IO(dio)
{
  printf("Gdey075T7() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEY075T7_WIDTH, GDEY075T7_HEIGHT, (int)GDEY075T7_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", (int) xPortGetFreeHeapSize());
}

void Gdey075T7::initPartialUpdate()
{
  IO.cmd(epd_panel_setting_partial.cmd);      // panel setting
  IO.data(epd_panel_setting_partial.data[0]); // partial update LUT from registers

  IO.cmd(0x82); // vcom_DC setting
  //      (0x2C);  // -2.3V same value as in OTP
  IO.data(0x26); // -2.0V
  //       (0x1C); // -1.5V
  IO.cmd(0x50);  // VCOM AND DATA INTERVAL SETTING
  IO.data(0x39); // LUTBD, N2OCP: copy new to old
  IO.data(0x07);

  // LUT Tables for partial update. Send them directly in 42 bytes chunks. In total 210 bytes
  IO.cmd(lut_20_LUTC_partial.cmd);
  IO.data(lut_20_LUTC_partial.data, lut_20_LUTC_partial.databytes);

  IO.cmd(lut_21_LUTWW_partial.cmd);
  IO.data(lut_21_LUTWW_partial.data, lut_21_LUTWW_partial.databytes);

  IO.cmd(lut_22_LUTKW_partial.cmd);
  IO.data(lut_22_LUTKW_partial.data, lut_22_LUTKW_partial.databytes);

  IO.cmd(lut_23_LUTWK_partial.cmd);
  IO.data(lut_23_LUTWK_partial.data, lut_23_LUTWK_partial.databytes);

  IO.cmd(lut_24_LUTKK_partial.cmd);
  IO.data(lut_24_LUTKK_partial.data, lut_24_LUTKK_partial.databytes);

  IO.cmd(lut_25_LUTBD_partial.cmd);
  IO.data(lut_25_LUTBD_partial.data, lut_25_LUTBD_partial.databytes);
}

//Initialize the display
void Gdey075T7::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdey075T7::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
  _wakeUp();
}

void Gdey075T7::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? GDEY075T7_8PIX_BLACK : GDEY075T7_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Gdey075T7::_wakeUp()
{
  IO.reset(10);
  //IMPORTANT: Some EPD controllers like to receive data byte per byte
  //So this won't work:
  //IO.data(epd_wakeup_power.data,epd_wakeup_power.databytes);
  IO.cmd(0x01);     //POWER SETTING
  IO.data(0x07);
  IO.data(0x07);   //VGH=20V,VGL=-20V
  IO.data(0x3f);   //VDH=15V
  IO.data(0x3f);   //VDL=-15V
  //Enhanced display drive(Add 0x06 command)
  IO.cmd(0x06);     //Booster Soft Start 
  IO.data(0x17);
  IO.data(0x17);   
  IO.data(0x28);   
  IO.data(0x17); 

  IO.cmd(0x04); //POWER ON
  //waiting for the electronic paper IC to release the idle signal
  _waitBusy("power_on");  

  IO.cmd(0X00);     //PANNEL SETTING
  IO.data(0x1F);   //KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  IO.cmd(0x61);         //tres      
  IO.data(0x03);   //source 800
  IO.data(0x20);
  IO.data(0x01);   //gate 480
  IO.data(0xE0);

  IO.cmd(0X15);   
  IO.data(0x00);    

  IO.cmd(0X50);     //VCOM AND DATA INTERVAL SETTING
  IO.data(0x10);
  IO.data(0x07);

  IO.cmd(0X60);     //TCON SETTING
  IO.data(0x22);
}

void Gdey075T7::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x13);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEY075T7_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= GDEY075T7_HEIGHT; y++)
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

uint16_t Gdey075T7::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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

void Gdey075T7::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Still in test mode\n");
  if (using_rotation)
    _rotate(x, y, w, h);
  if (x >= GDEY075T7_WIDTH)
    return;
  if (y >= GDEY075T7_HEIGHT)
    return;
  uint16_t xe = gx_uint16_min(GDEY075T7_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEY075T7_HEIGHT, y + h) - 1;

  // x &= 0xFFF8; // byte boundary, need to test this
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) {
    _wakeUp();
    }

  _using_partial_mode = true;
  initPartialUpdate();

  {               // leave both controller buffers equal
    IO.cmd(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.cmd(0x13);

    for (int16_t y1 = y; y1 <= ye; y1++)
    {
      for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
      {
        uint16_t idx = y1 * (GDEY075T7_WIDTH / 8) + x1;
        // white is 0x00 in buffer
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        // white is 0xFF on device
        IO.data(data);

        if (idx % 8 == 0)
        {
          #if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
          rtc_wdt_feed();
          #endif
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      }
    }
    IO.cmd(0x12); // display refresh
    _waitBusy("updateWindow");
    IO.cmd(0x92); // partial out
  }

  vTaskDelay(GDEY075T7_PU_DELAY / portTICK_PERIOD_MS);
}

void Gdey075T7::_waitBusy(const char *message)
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

void Gdey075T7::_sleep()
{
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xA5);
}

void Gdey075T7::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEY075T7_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEY075T7_WIDTH - x - w - 1;
    y = GDEY075T7_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEY075T7_HEIGHT - y - h - 1;
    break;
  }
}

void Gdey075T7::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEY075T7_WIDTH - x - 1;
    break;
  case 2:
    x = GDEY075T7_WIDTH - x - 1;
    y = GDEY075T7_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEY075T7_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * GDEY075T7_WIDTH / 8;

  if (color)
  {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  }
  else
  {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  }
}

#include "wave7i5.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

#define GDEW075T7_PU_DELAY 100

// Constructor
Wave7i5::Wave7i5(EpdSpi &dio) : Adafruit_GFX(GDEW075T7_WIDTH, GDEW075T7_HEIGHT),
                                    Epd(GDEW075T7_WIDTH, GDEW075T7_HEIGHT), IO(dio)
{
  printf("Wave7i5() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075T7_WIDTH, GDEW075T7_HEIGHT, GDEW075T7_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

//Initialize the display
void Wave7i5::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Wave7i5::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
}

void Wave7i5::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? GDEW075T7_8PIX_BLACK : GDEW075T7_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Wave7i5::_wakeUp()
{
  IO.reset(10);

  IO.cmd(0x01); 
  IO.data(0x07);
  IO.data(0x07);
  IO.data(0x3f);
  IO.data(0x3f);

  IO.cmd(0x04);
  _waitBusy("Power on");
  
  IO.cmd(0X00);			// PANNEL SETTING
  IO.data(0x1F);    // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

  IO.cmd(0x61);    	// Tres
  IO.data(0x03);		// source 800
  IO.data(0x20);
  IO.data(0x01);		// gate 480
  IO.data(0xE0);

  IO.cmd(0X15);
  IO.data(0x00);

  IO.cmd(0X50);			// VCOM AND DATA INTERVAL SETTING
  IO.data(0x10);
  IO.data(0x07);

  IO.cmd(0X60);			// TCON SETTING
  IO.data(0x22);
}

void Wave7i5::update()
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

  _sleep();
}

void Wave7i5::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Waveshare this is not a good example of partial update. Use Gdew075T7\n");
  /* if (using_rotation)
    _rotate(x, y, w, h);
  if (x >= GDEW075T7_WIDTH)
    return;
  if (y >= GDEW075T7_HEIGHT)
    return;

  if (!_using_partial_mode)
  _wakeUp();
  _using_partial_mode = true;

  IO.cmd(0x13);
  for (unsigned long j = 0; j < GDEW075T7_HEIGHT; j++) {
    for (unsigned long i = 0; i < GDEW075T7_WIDTH/8; i++) {
        if( (j>=y) && (j<y+h) && (i*8>=x) && (i*8<x+w)){
            IO.data(~ _buffer[i-x/8 + (w)/8*(j-y)] );
        }else {
            IO.data(0x00);
        }
    }
  }
  
  IO.cmd(0x12);
  
  _waitBusy("updateWindow");
  
  vTaskDelay(GDEW075T7_PU_DELAY / portTICK_PERIOD_MS); */
}

void Wave7i5::_waitBusy(const char *message)
{
  IO.cmd(0x71); // Not sure if this is needed. In Wave example is on the loop (?)
  if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // wait until busy goes High
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

void Wave7i5::_sleep()
{
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xA5);
}

void Wave7i5::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
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

void Wave7i5::drawPixel(int16_t x, int16_t y, uint16_t color)
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

  if (!color)
  {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  }
  else
  {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
  }
}

uint16_t Wave7i5::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  printf("Not used in Wave7i5 EPD\n");
  return 0;
}
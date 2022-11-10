// Epaper: 4.01inch ACeP 7-Color https://www.waveshare.com/4.01inch-e-paper-hat-f.htm | Wiki: https://github.com/martinberlin/cale-idf/wiki/Model-color-wave5i7color.h
// This epaper like most color models does not support partialUpdate
// This epaper should be used only on warm environments others colors are not intense at all. Check Wiki
#include "color/wave4i7Color.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Wave4i7Color::Wave4i7Color(EpdSpi& dio): 
  Adafruit_GFX(WAVE4I7COLOR_WIDTH, WAVE4I7COLOR_HEIGHT),
  Epd7Color(WAVE4I7COLOR_WIDTH, WAVE4I7COLOR_HEIGHT), IO(dio)
{
  printf("Wave4i7Color() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  WAVE4I7COLOR_WIDTH, WAVE4I7COLOR_HEIGHT);  
}

//Initialize the display
void Wave4i7Color::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Wave4i7Color::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz 

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Wave4i7Color::fillScreen(uint16_t color)
{
  uint8_t pv = _color7(color);
  uint8_t pv2 = pv | pv << 4;
  for (uint32_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = pv2;
  }

  if (debug_enabled) printf("fillScreen(%x) black/red _buffer len:%d\n", color, sizeof(_buffer));
}

void Wave4i7Color::_wakeUp(){
  IO.reset(1);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  // Wait for the electronic paper IC to release the idle signal
  _waitBusy("epd_wakeup reset");

  // <Essential settings> in Waveshare's example 
  IO.cmd(0X00); //PANNEL SETTING
  IO.data(0x2F);
  IO.data(0x00);

  IO.cmd(0x01); //POWER SETTING
  IO.data(0x37); 
  IO.data(0x00);
  IO.data(0x05);
  IO.data(0x05);
  // </Essential settings>
  
  IO.cmd(0x03); //Unknown
  IO.data(0x00);

  // Additional settings proposed by Jean-Marc in GxEPD2 (Do not see any visual difference)
  IO.cmd(0x06); // Booster Soft Start
  IO.data(0xC7);
  IO.data(0xC7);
  IO.data(0x1D);

  //IO.cmd(0x30); // PLL Control
  //IO.data(0x3C);    // 50 Hz
  IO.cmd(0x41);
  IO.data(0x00);

  IO.cmd(0x50); // VCOM and Data Interval Setting
  IO.data(0x37);    // white border

  IO.cmd(0x60); // TCON 
  IO.data(0x22);

  // <Essential settings> in Waveshare's example 
  IO.cmd(0x61);  // Resolution setting 600*448
  IO.data(0x02); //source 600
  IO.data(0x80);
  IO.data(0x01); //gate
  IO.data(0x90);
  // </Essential settings>

  IO.cmd(0xE3);  //PWS
  IO.data(0xAA);
}

void Wave4i7Color::update()
{
  printf("display.update() called\n");

  uint64_t startTime = esp_timer_get_time();
  _wakeUp();

  IO.cmd(0x10);

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  if (spi_optimized) {
    uint32_t i = 0;
    uint16_t xLineBytes = WAVE4I7COLOR_WIDTH/2;
    uint8_t x1buf[xLineBytes];
    for (uint16_t y = 1; y <= WAVE4I7COLOR_HEIGHT; y++)
    {
      for (uint16_t x = 1; x <= xLineBytes; x++)
      {
        uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x33;
        x1buf[x - 1] = data;
        if (x == xLineBytes)
        { // Flush the X line buffer to SPI
          IO.data(x1buf, sizeof(x1buf));
        }
        ++i;
      }
    }
    if (debug_enabled) {
      printf("\nSPI optimization is on. Sending full xLineBytes: %d per SPI (4 bits per pixel)\n\nBuffer size: %d  expected size: %d\n", 
     (int) xLineBytes, (int) i, (int) WAVE4I7COLOR_BUFFER_SIZE);
    }

  } else {
    for (uint32_t i = 0; i < sizeof(_buffer); i++) {
      IO.data(_buffer[i]);
    }
  }
  
  IO.cmd(0x04); // Power on
  _waitBusy("Power on");

  uint64_t endTime = esp_timer_get_time();

  IO.cmd(0x12);
  _waitBusy("0x12 display refresh");

  uint64_t powerOnTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  // DEBUG Disable sleep until Buffer is completely written and tested
  //vTaskDelay(1000 / portTICK_PERIOD_MS);
  //_sleep();
}

void Wave4i7Color::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Wave4i7Color::_sleep() {
  IO.cmd(0x02);
  _waitBusy("poweroff");

  IO.cmd(0x07); // deep sleep
  IO.data(0xA5);
}

void Wave4i7Color::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = WAVE4I7COLOR_WIDTH - x - w - 1;
      break;
    case 2:
      x = WAVE4I7COLOR_WIDTH - x - w - 1;
      y = WAVE4I7COLOR_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = WAVE4I7COLOR_HEIGHT - y - h - 1;
      break;
  }
}

/**
 * From GxEPD2 (Jean-Marc)
 */
void Wave4i7Color::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = WAVE4I7COLOR_WIDTH - x - 1;
      break;
    case 2:
      x = WAVE4I7COLOR_WIDTH - x - 1;
      y = WAVE4I7COLOR_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = WAVE4I7COLOR_HEIGHT - y - 1;
      break;
  }
  uint32_t i = x / 2 + uint32_t(y) * (WAVE4I7COLOR_WIDTH / 2);
  uint8_t pv = _color7(color);
      
  if (x & 1) _buffer[i] = (_buffer[i] & 0xF0) | pv;
    else _buffer[i] = (_buffer[i] & 0x0F) | (pv << 4);
}

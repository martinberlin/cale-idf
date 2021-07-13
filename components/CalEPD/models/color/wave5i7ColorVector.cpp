// Epaper: 5.65inch ACeP 7-Color  https://www.waveshare.com/product/displays/e-paper/5.65inch-e-paper-module-f.htm
// This epaper like most color models does not support partialUpdate
#include "color/wave5i7ColorVector.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Wave5i7Color::Wave5i7Color(EpdSpi& dio): 
  Adafruit_GFX(WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT),
  Epd7Color(WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT), IO(dio)
{
  printf("Wave5i7Color() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT);
  _buffer.reserve(WAVE5I7COLOR_BUFFER_SIZE);
}

//Initialize the display
void Wave5i7Color::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Wave5i7Color::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz 

    //Reset the display
    IO.reset(20);
    //Initialize _buffer
    for (uint32_t x = 0; x < WAVE5I7COLOR_BUFFER_SIZE; x++)
    {
      _buffer.push_back(0xff);
    }

    fillScreen(EPD_WHITE);
}

void Wave5i7Color::fillScreen(uint16_t color)
{
  uint8_t pv = _color7(color);
  uint8_t pv2 = pv | pv << 4;
  printf("BUFFER_SIZE %d\n",WAVE5I7COLOR_BUFFER_SIZE);
  for (uint32_t x = 0; x < WAVE5I7COLOR_BUFFER_SIZE; x++)
  {
    buffer_it = _buffer.begin()+x;
    *(buffer_it) = pv2;
    //printf("%d ",x);
  }

  if (debug_enabled) printf("fillScreen(%x) _buffer len:%d\n", color, _buffer.size());
}

void Wave5i7Color::_wakeUp(){
  IO.reset(1);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  // Wait for the electronic paper IC to release the idle signal
  _waitBusy("epd_wakeup reset");

  // <Essential settings> in Waveshare's example 
  IO.cmd(0X00); //PANNEL SETTING
  IO.data(0xEF);
  IO.data(0x08);

  IO.cmd(0x01); //POWER SETTING
  IO.data(0x37); 
  IO.data(0x00);
  IO.data(0x23);
  IO.data(0x23);
  // </Essential settings>
  
  IO.cmd(0x03); //Unknown
  IO.data(0x00);

  // Additional settings proposed by Jean-Marc in GxEPD2 (Do not see any visual difference)
  IO.cmd(0x06); // Booster Soft Start
  IO.data(0xC7);
  IO.data(0xC7);
  IO.data(0x1D);

  IO.cmd(0x30); // PLL Control
  IO.data(0x3C);    // 50 Hz

  IO.cmd(0x40); // Temperature Sensor Command
  IO.data(0x00);

  IO.cmd(0x50); // VCOM and Data Interval Setting
  IO.data(0x37);// white border

  IO.cmd(0x60); // TCON
  IO.data(0x22);

  // <Essential settings> in Waveshare's example 
  IO.cmd(0x61);  // Resolution setting 600*448
  IO.data(0x02); //source 600
  IO.data(0x58);
  IO.data(0x01); //gate 448
  IO.data(0xc0);
  // </Essential settings>

  IO.cmd(0xE3); //PWS
  IO.data(0xAA);

  // Not sure why it's needed to do this and send VCOM again
  vTaskDelay(100 / portTICK_PERIOD_MS);
  IO.cmd(0x50);  // VCOM and Data Interval Setting
  IO.data(0x37);
}

void Wave5i7Color::update()
{
  printf("display.update() called\n");

  uint64_t startTime = esp_timer_get_time();
  _wakeUp();

  IO.cmd(0x10);

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  if (spi_optimized) {
    uint32_t i = 0;
    uint16_t xLineBytes = WAVE5I7COLOR_WIDTH/2;
    uint8_t x1buf[xLineBytes];
    for (uint16_t y = 1; y <= WAVE5I7COLOR_HEIGHT; y++)
    {
      for (uint16_t x = 1; x <= xLineBytes; x++)
      {
        uint8_t data = i < _buffer.size() ? _buffer.at(i) : 0x33;
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
     xLineBytes, i, WAVE5I7COLOR_BUFFER_SIZE);
    }

  } else {
    for (uint32_t i = 0; i < _buffer.size(); i++) {
      IO.data(_buffer.at(i));
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

void Wave5i7Color::_waitBusy(const char* message){
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

void Wave5i7Color::_sleep() {
  IO.cmd(0x02);
  _waitBusy("poweroff");

  IO.cmd(0x07); // deep sleep
  IO.data(0xA5);
}

void Wave5i7Color::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = WAVE5I7COLOR_WIDTH - x - w - 1;
      break;
    case 2:
      x = WAVE5I7COLOR_WIDTH - x - w - 1;
      y = WAVE5I7COLOR_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = WAVE5I7COLOR_HEIGHT - y - h - 1;
      break;
  }
}

/**
 * From GxEPD2 (Jean-Marc)
 */
void Wave5i7Color::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = WAVE5I7COLOR_WIDTH - x - 1;
      break;
    case 2:
      x = WAVE5I7COLOR_WIDTH - x - 1;
      y = WAVE5I7COLOR_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = WAVE5I7COLOR_HEIGHT - y - 1;
      break;
  }
  uint32_t pos = x / 2 + uint32_t(y) * (WAVE5I7COLOR_WIDTH / 2);
  uint8_t pv = _color7(color);
      
      // #43 TODO: Check why is trying to update out of bonds anyways
  if (pos >= _buffer.size()) {
    if (_vec_bonds_check) {
      printf("x:%d y:%d Vpos:%d >out bonds\n",x,y, pos);
      _vec_bonds_check = false;
    }
    return;
  }
  buffer_it = _buffer.begin()+pos;
  if (x & 1) *(buffer_it) = (_buffer.at(pos) & 0xF0) | pv;
    else *(buffer_it) = (_buffer.at(pos) & 0x0F) | (pv << 4);
}

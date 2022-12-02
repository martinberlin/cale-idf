// This epaper like most color models does not support partialUpdate
#include "color/gdey073d46.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Constructor
gdey073d46::gdey073d46(EpdSpi& dio): 
  Adafruit_GFX(GDEY073D46_WIDTH, GDEY073D46_HEIGHT),
  Epd7Color(GDEY073D46_WIDTH, GDEY073D46_HEIGHT), IO(dio)
{
  printf("gdey073d46() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEY073D46_WIDTH, GDEY073D46_HEIGHT);  
}

//Initialize the display
void gdey073d46::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("gdey073d46::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz 

    //Reset the display
    IO.reset(10);
    fillScreen(EPD_WHITE);
}

void gdey073d46::fillScreen(uint16_t color)
{
  uint8_t pv = _color7(color);
  uint8_t pv2 = pv | pv << 4;
  for (uint32_t x = 0; x < GDEY073D46_BUFFER_SIZE; x++)
  {
    _buffer[x] = pv2;
  }

  if (debug_enabled) printf("fillScreen(%x) _buffer len:%d\n", color, sizeof(_buffer));
}

void gdey073d46::_wakeUp(){
  IO.reset(10);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  // Wait for the electronic paper IC to release the idle signal
  //_waitBusy("epd_wakeup reset");

  // <Essential settings> in GOODISPLAY example 
  IO.cmd(0xAA);   // CMDH
  IO.data(0x49);
  IO.data(0x55);
  IO.data(0x20);
  IO.data(0x08);
  IO.data(0x09);
  IO.data(0x18);

  IO.cmd(0x01);   // PWRR
  IO.data(0x3F);
  IO.data(0x00);
  IO.data(0x32);
  IO.data(0x2A);
  IO.data(0x0E);
  IO.data(0x2A);
  
  IO.cmd(0x00);   // PSR
  IO.data(0x5F);
  IO.data(0x69);
  
  IO.cmd(0x03);   // POFS
  IO.data(0x00);
  IO.data(0x54);
  IO.data(0x00);
  IO.data(0x44); 

  IO.cmd(0x05);   // BTST1
  IO.data(0x40);
  IO.data(0x1F);
  IO.data(0x1F);
  IO.data(0x2C);
  
  IO.cmd(0x06);   // BTST2
  IO.data(0x6F);
  IO.data(0x1F);
  IO.data(0x16);
  IO.data(0x25);

  IO.cmd(0x08);   // BTST3
  IO.data(0x6F);
  IO.data(0x1F);
  IO.data(0x1F);
  IO.data(0x22);
 
  IO.cmd(0x13);    // IPC
  IO.data(0x00);
  IO.data(0x04);
  
  IO.cmd(0x30);    // PLL
  IO.data(0x02);
  
  IO.cmd(0x41);    // TSE
  IO.data(0x00);
  
  IO.cmd(0x50);    // CDI
  IO.data(0x3F);
  
  IO.cmd(0x60);    // TCON
  IO.data(0x02);
  IO.data(0x00);
  
  IO.cmd(0x61);    // TRES
  IO.data(0x03);
  IO.data(0x20);
  IO.data(0x01); 
  IO.data(0xE0);
  
  IO.cmd(0x82);    // VDCS
  IO.data(0x1E); 

  IO.cmd(0x84);    // T_VDCS
  IO.data(0x00);

  IO.cmd(0x86);    // AGID
  IO.data(0x00);
  
  IO.cmd(0xE3);    // PWS
  IO.data(0x2F);
 
  IO.cmd(0xE0);    // CCSET
  IO.data(0x00); 
  
  IO.cmd(0xE6);   // TSSET
  IO.data(0x00);	

	IO.cmd(0x04);		//PWR on
  _waitBusy("power on");
}

void gdey073d46::update()
{
  printf("display.update() called\n");

  uint64_t startTime = esp_timer_get_time();
  _wakeUp();

  IO.cmd(0x10);

  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  if (spi_optimized) {
    uint32_t i = 0;
    uint16_t xLineBytes = GDEY073D46_WIDTH/2;
    uint8_t x1buf[xLineBytes];
    for (uint16_t y = 1; y <= GDEY073D46_HEIGHT; y++)
    {
      for (uint16_t x = 1; x <= xLineBytes; x++)
      {
        uint8_t data = i < GDEY073D46_BUFFER_SIZE ? _buffer[i] : 0x33;
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
      (int)xLineBytes, (int)i, (int)GDEY073D46_BUFFER_SIZE);
    }

  } else {
    for (uint32_t i = 0; i < GDEY073D46_BUFFER_SIZE; i++) {
      IO.data(_buffer[i]);
    }
  }

  uint64_t endTime = esp_timer_get_time();

  IO.cmd(0x12);
  IO.data(0x00);
  vTaskDelay(2);
  _waitBusy("0x12 display refresh");

  uint64_t powerOnTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  // DEBUG Disable sleep until Buffer is completely written and tested
  //vTaskDelay(1000 / portTICK_PERIOD_MS);
  _sleep();
}

void gdey073d46::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
      ESP_LOGI(TAG, "Busy release for %s in %llu ms", message, (esp_timer_get_time()-time_since_boot)/1000 );
      break;
    }
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout: %s", message);
      break;
    }
  }
}

void gdey073d46::_sleep() {
  IO.cmd(0x02);
  // deep sleep
  IO.data(0x00);
  _waitBusy("poweroff");
}

void gdey073d46::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEY073D46_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEY073D46_WIDTH - x - w - 1;
      y = GDEY073D46_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEY073D46_HEIGHT - y - h - 1;
      break;
  }
}

/**
 * From GxEPD2 (Jean-Marc)
 */
void gdey073d46::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEY073D46_WIDTH - x - 1;
      break;
    case 2:
      x = GDEY073D46_WIDTH - x - 1;
      y = GDEY073D46_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEY073D46_HEIGHT - y - 1;
      break;
  }
  uint32_t i = x / 2 + uint32_t(y) * (GDEY073D46_WIDTH / 2);
  uint8_t pv = _color7(color);
      
  if (x & 1) _buffer[i] = (_buffer[i] & 0xF0) | pv;
    else _buffer[i] = (_buffer[i] & 0x0F) | (pv << 4);
}

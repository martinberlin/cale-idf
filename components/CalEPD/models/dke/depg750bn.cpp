
#include "dke/depg750bn.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#define ENABLE		1
#define DISABLE		2

const uint8_t Depg750bn::LUTDefault_VCOM[] = {
    0x20,
    0x00,    0x00,    0x14,   0x00,     0x00,    0x01,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
};
const uint8_t Depg750bn::LUTDefault_LUTWW[] = {
    0x21,
    0x00,   0x00,     0x14,    0x00,     0x00,    0x01,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
};
const uint8_t Depg750bn::LUTDefault_LUTBW[] = {
    0x22,
    0x20,   0x00,    0x14,   0x00,      0x00,    0x01,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
};
const uint8_t Depg750bn::LUTDefault_LUTWB[] = {
    0x23,
    0x10,    0x00,    0x14,   0x00,    0x00,    0x01,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
};
const uint8_t Depg750bn::LUTDefault_LUTBB[] = {
    0x24,
    0x00,    0x00,    0x14,   0x00,    0x00,    0x01,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
};



// Constructor
Depg750bn::Depg750bn(EpdSpi& dio):
  Adafruit_GFX(DEPG750BN_WIDTH, DEPG750BN_HEIGHT),
  Epd(DEPG750BN_WIDTH, DEPG750BN_HEIGHT), IO(dio)
{
  printf("Depg750bn() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  DEPG750BN_WIDTH, DEPG750BN_HEIGHT);
}

//Initialize the display
void Depg750bn::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Depg750bn::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, ENABLE);

    IO.init(4, debug);
    fillScreen(EPD_WHITE);
}

void Depg750bn::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? DEPG750BN_8PIX_BLACK : DEPG750BN_8PIX_WHITE;

  for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
  {
	  _mono_buffer[x] = data;
  }
}

void Depg750bn::_wakeUp(){

	IO.reset(10);
	//  IO.cmd(0x12); // SWRESET
	// Theoretically this display could be driven without RST pin connected
	_waitBusy("SWRESET");

	IO.cmd		(0x01); 					// POWER SETTING
	IO.data 	(0x07);
	IO.data 	(0x07); 					// VGH=20V,VGL=-20V
	IO.data 	(0x3f); 					// VDH=15V
	IO.data 	(0x3f); 					// VDL=-15V
	IO.cmd		(0x04);
	_waitBusy	("_wakeUp Power On");
	IO.cmd		(0x00); 					//PANEL SETTING
	IO.data		(0x1f); 					//KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f
	IO.cmd		(0x61); 					//tres
	IO.data 	(DEPG750BN_WIDTH / 256); 	//source 800
	IO.data 	(DEPG750BN_WIDTH % 256);
	IO.data 	(DEPG750BN_HEIGHT / 256); 	//gate 480
	IO.data 	(DEPG750BN_HEIGHT % 256);
	IO.cmd		(0x15);
	IO.data		(0x00);
	IO.cmd		(0x50); 					//VCOM AND DATA INTERVAL SETTING
	IO.data		(0x29);    					// LUTKW, N2OCP: copy new to old
	IO.data		(0x07);
	IO.cmd		(0x60); 					//TCON SETTING
	IO.data		(0x22);
	IO.cmd		(0x00); 					// panel setting
	IO.data		(0x1f);    					// full update LUT from OTP

}

void Depg750bn::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
    // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = DEPG750BN_WIDTH/8;
  uint8_t x1buf[xLineBytes];

  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n", (int)DEPG750BN_BUFFER_SIZE);

  for(uint32_t i =  0; i <= DEPG750BN_BUFFER_SIZE; i++)
  {
	  IO.data(DEPG750BN_8PIX_WHITE);
  }
  IO.cmd(0x13);

  for(uint32_t i =  0; i <= DEPG750BN_BUFFER_SIZE; i++)
  {
	  IO.data((i < sizeof(_mono_buffer)) ? _mono_buffer[i] : 0x00);
  }
  IO.cmd(0x12);
  _waitBusy("update");
  _sleep();


}

void Depg750bn::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("Color epapers from Goodisplay do not support partial update. Full update triggered\n");
  update();
}

void Depg750bn::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // In this controller BUSY == 0
  while (true){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Depg750bn::_sleep(){
  IO.cmd(0X02); //power off
  _waitBusy("Power Off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xa5);
}

void Depg750bn::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = DEPG750BN_WIDTH - x - w - 1;
      break;
    case 2:
      x = DEPG750BN_WIDTH - x - w - 1;
      y = DEPG750BN_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = DEPG750BN_HEIGHT - y - h - 1;
      break;
  }
}

void Depg750bn::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = DEPG750BN_WIDTH - x - 1;
      break;
    case 2:
      x = DEPG750BN_WIDTH - x - 1;
      y = DEPG750BN_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = DEPG750BN_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * DEPG750BN_WIDTH / 8;
  // In this display controller RAM colors are inverted: WHITE RAM(BW) = 1  / BLACK = 0
  switch (color)
  {
  case EPD_BLACK:
    color = EPD_WHITE;
    break;
  case EPD_WHITE:
    color = EPD_BLACK;
    break;
  }

  _mono_buffer[i] = (_mono_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white

  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _mono_buffer[i] = (_mono_buffer[i] | (1 << (7 - x % 8)));

}

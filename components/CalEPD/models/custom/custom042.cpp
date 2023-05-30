#include "custom/custom042.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Constructor
Custom042::Custom042(EpdSpi& dio): 
  Adafruit_GFX(CUSTOM042_WIDTH, CUSTOM042_HEIGHT),
  Epd(CUSTOM042_WIDTH, CUSTOM042_HEIGHT), IO(dio)
{
  printf("Custom042() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  CUSTOM042_WIDTH, CUSTOM042_HEIGHT);  
}

//Initialize the display
void Custom042::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Custom042::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Custom042::_lut_GC()
{
	IO.cmd(0x20);							//vcom
	IO.data(Custom042::lut_R20_GC, sizeof(Custom042::lut_R20_GC));
	IO.cmd(0x21);							// Required * Otherwise all gray
	IO.data(Custom042::lut_R21_GC, sizeof(Custom042::lut_R21_GC));
	IO.cmd(0x24);							//bb b
	IO.data(Custom042::lut_R22_GC, sizeof(Custom042::lut_R22_GC));
	IO.cmd(0x22);							//bw r
  IO.data(Custom042::lut_R23_GC, sizeof(Custom042::lut_R23_GC));
	IO.cmd(0x23);							//wb w
  IO.data(Custom042::lut_R24_GC, sizeof(Custom042::lut_R24_GC));
}

void Custom042::_wakeUp(){

  IO.reset(10);
  
  IO.cmd(0x00); 		//Panel settings
	IO.data(0x3F);
  //IO.data(0x37); //0x37 -> Lo da vuelta
	//IO.data(0x8A);

	IO.cmd(0x01);     	//Power settings
	IO.data(0x03);
	IO.data(0x10);
	IO.data(0x3f);
	IO.data(0x3f);
	IO.data(0x03);

	IO.cmd(0x06);      	// Booster soft start
	IO.data(0xE7); //A 0xE7
	IO.data(0xE7); //B 0xE7
	IO.data(0x3D); //C 0x3D

	IO.cmd(0x04); // Power on
	_waitBusy("epd_power on");;

	IO.cmd(0X60);      	// TCON SETTING  (TCON)
	IO.data(0x22);

	IO.cmd(0x82);     	// vcom
	IO.data(0x00);

	IO.cmd(0x30); 		// PLL control
	IO.data(0x09);

	IO.cmd(0xE3);      	// Power saving
	IO.data(0x88);

	IO.cmd(0x61);      	// Resolution setting
	IO.data(0x01);
	IO.data(0x90);
	IO.data(0x01);
	IO.data(0x2C);
}

void Custom042::_waitBusy(const char* message){
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

void Custom042::_sleep(){
    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING  
    IO.data(0xf7);
       
    IO.cmd(0X02);   //power off
    _waitBusy("power off");
    IO.cmd(0X07);   //deep sleep
    IO.data(0xA5);
}

void Custom042::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = CUSTOM042_WIDTH - x - w - 1;
      break;
    case 2:
      x = CUSTOM042_WIDTH - x - w - 1;
      y = CUSTOM042_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = CUSTOM042_HEIGHT - y - h - 1;
      break;
  }
}

void Custom042::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
  _lut_GC();

  // BLACK: Write RAM for black(0)/white (1)
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = CUSTOM042_WIDTH/8;
  uint8_t x1buf[xLineBytes];

// Note that in IC specs is 0x10 BLACK and 0x13 RED
// BLACK: Write RAM
  IO.cmd(0x13);
  for(uint16_t y =  1; y <= CUSTOM042_HEIGHT; y++) {
      for(uint16_t x = 1; x <= xLineBytes; x++) {
        x1buf[x-1] = _black_buffer[i];
        if (x == xLineBytes) { // Flush the X line buffer to SPI
          IO.data(x1buf,sizeof(x1buf));
        }
        ++i;
      }
  }

   i = 0;

  // RED: Write RAM1
  IO.cmd(0x10);
    for(uint16_t y =  1; y <= CUSTOM042_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          x1buf[x-1] = 0xFF;
          if (x==xLineBytes) {
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);     //DISPLAY REFRESH 

  _waitBusy("epaper refresh");
  uint64_t powerOnTime = esp_timer_get_time();

  // GxEPD comment: Avoid double full refresh after deep sleep wakeup
  // if (_initial) {  // --> Original deprecated 
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

void Custom042::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = CUSTOM042_WIDTH - x - 1;
      break;
    case 2:
      x = CUSTOM042_WIDTH - x - 1;
      y = CUSTOM042_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = CUSTOM042_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * CUSTOM042_WIDTH / 8;
  
  // This formulas are from gxEPD that apparently got the color right:
  _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8))); // white pixel
  _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));     // white pixel

  if (color == EPD_WHITE) {
    //printf("w:%x ",_black_buffer[i]);
    return;
  }
  else if (color == EPD_BLACK) {
    _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    //printf("B ");
  }
  else if (color == EPD_RED) {
    _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    //printf("R %x ",_red_buffer[i]);
  }
}


void Custom042::fillScreen(uint16_t color)
{
  // Fill screen will be inverted with the way is done NOW
  uint8_t black = CUSTOM042_8PIX_WHITE;
  uint8_t red = CUSTOM042_8PIX_RED_WHITE;
  if (color == EPD_WHITE) {
    if (debug_enabled) printf("fillScreen WHITE\n");
  } else if (color == EPD_BLACK) {
    black = CUSTOM042_8PIX_BLACK;
    if (debug_enabled) printf("fillScreen BLACK SELECTED\n");
  } else if (color == EPD_RED) {
    red = CUSTOM042_8PIX_RED;
    if (debug_enabled) printf("fillScreen RED SELECTED\n");
  } else if ((color & 0xF100) > (0xF100 / 2)) {
    red = 0xFF;
  } else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) {
    black = 0xFF;
  }
  
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }

  if (debug_enabled) printf("fillScreen(%x) black/red _buffer len:%d\n",color,sizeof(_black_buffer));
}

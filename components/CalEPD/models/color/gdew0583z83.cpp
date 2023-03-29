//Controller: UC8179 (B/R/W) https://github.com/GoodDisplay/E-paper-Display-Library-of-GoodDisplay/blob/main/Tri-Color_E-paper-Display/5.83inch_UC8179_GDEW0583Z83_648x480/ESP32-Arduino%20IDE/GDEW0583Z83_ESP32.ino
#include "color/gdew0583z83.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// CMD, DATA, Databytes * Optional we are going to use sizeof(data)
DRAM_ATTR const epd_init_4 Gdew0583z83::epd_wakeup_power={
0x01,{
  0x07,
  0x07, //VGH=20V,VGL=-20V
  0x3F, //VDH=15V
  0x3F  //VDL=15V
  },4
};

DRAM_ATTR const epd_init_1 Gdew0583z83::epd_panel_setting={
0x00,{
  0x0F},1
};

DRAM_ATTR const epd_init_4 Gdew0583z83::epd_resolution={
0x61,{
  0x02, //source 600
  0x88,
  0x01, //gate 448
  0xE0
},4};

// Constructor
Gdew0583z83::Gdew0583z83(EpdSpi& dio): 
  Adafruit_GFX(GDEW0583Z83_WIDTH, GDEW0583Z83_HEIGHT),
  Epd(GDEW0583Z83_WIDTH, GDEW0583Z83_HEIGHT), IO(dio)
{
  printf("Gdew0583z83() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0583Z83_WIDTH, GDEW0583Z83_HEIGHT);  
}

//Initialize the display
void Gdew0583z83::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0583z83::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug
    IO.init(4, debug);
    fillScreen(EPD_WHITE);
}

void Gdew0583z83::fillScreen(uint16_t color)
{
  uint8_t black = GDEW0583Z83_8PIX_WHITE;
  uint8_t red = GDEW0583Z83_8PIX_RED_WHITE;
  if (color == EPD_RED) {

  } else if (color == EPD_BLACK) {
    black = GDEW0583Z83_8PIX_BLACK;
    printf("fillScreen BLACK SELECTED\n");
  } else if (color == EPD_WHITE) {
    red = GDEW0583Z83_8PIX_RED;
    printf("fillScreen EPD_WHITE\n");
  } else if ((color & 0xF100) > (0xF100 / 2)) {
    red = 0xFF;
    printf("fillScreen RED 0xFF\n");
  } else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) {
    black = 0xFF;
    printf("fillScreen BLACK 0xFF\n");
  }
  
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void Gdew0583z83::_wakeUp(){
  IO.reset(10);
  IO.cmd(epd_wakeup_power.cmd);
 
  for (int i=0;i<sizeof(epd_wakeup_power.data);++i) {
    IO.data(epd_wakeup_power.data[i]);
  }

    // Power it on
  IO.cmd(0x04);
  _waitBusy("Power on");
  
  IO.cmd(epd_panel_setting.cmd);
  IO.data(epd_panel_setting.data[0]); //KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  IO.cmd(epd_resolution.cmd); // tres
  for (int i=0;i<sizeof(epd_resolution.data);++i) {
    IO.data(epd_resolution.data[i]);
  }

  IO.cmd(0x06); //BOOST makes red more red
  IO.data(0xC7);
  IO.data(0xCC); 
  IO.data(0x28);

  IO.cmd(0x15);
  IO.data(0x00);

	IO.cmd(0X50);			//VCOM AND DATA INTERVAL SETTING
	IO.data(0x11);
  IO.data(0x07);

	IO.cmd(0X60);			//TCON SETTING
	IO.data(0x22);

}

void Gdew0583z83::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
    // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW0583Z83_WIDTH/8;
  uint8_t x1buf[xLineBytes];

  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n", (int)GDEW0583Z83_BUFFER_SIZE);  
  
  for(uint16_t y =  1; y <= GDEW0583Z83_HEIGHT; y++) {
    for(uint16_t x = 1; x <= xLineBytes; x++) {
      uint8_t data = _black_buffer[i];
      x1buf[x-1] = data;
      if (x==xLineBytes) { // Flush the X line buffer to SPI
        IO.data(x1buf,sizeof(x1buf));
      }
      ++i;
    }
  }
  i = 0;
  
  IO.cmd(0x13); // Red
  for(uint16_t y =  1; y <= GDEW0583Z83_HEIGHT; y++) {
      for(uint16_t x = 1; x <= xLineBytes; x++) {
        uint8_t data = _red_buffer[i];
        x1buf[x-1] = data;
        if (x==xLineBytes) { // Flush the X line buffer to SPI
          IO.data(x1buf,sizeof(x1buf));
        }
        ++i;
      }
    }

  IO.cmd(0x12);
  
  uint64_t endTime = esp_timer_get_time();
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);
  _sleep();
}

void Gdew0583z83::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("Color epapers from Goodisplay do not support partial update. Full update triggered\n");
  update();
}

void Gdew0583z83::_waitBusy(const char* message){
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

void Gdew0583z83::_sleep(){
  IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING  
  IO.data(0xf7);
       
  IO.cmd(0X02); //power off
  _waitBusy("Power Off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xa5);
}

void Gdew0583z83::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0583Z83_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0583Z83_WIDTH - x - w - 1;
      y = GDEW0583Z83_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0583Z83_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0583z83::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0583Z83_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0583Z83_WIDTH - x - 1;
      y = GDEW0583Z83_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0583Z83_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0583Z83_WIDTH / 8;
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

  _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else if (color == EPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}


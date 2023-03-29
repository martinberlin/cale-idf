//Controller: IL0371 (3 colors) http://www.e-paper-display.com/download_detail/downloadsId%3d536.html
#include "color/gdew0583z21.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// CMD, DATA, Databytes * Optional we are going to use sizeof(data)
DRAM_ATTR const epd_init_2 Gdew0583z21::epd_wakeup_power={
0x01,{
  0x37,
  0x00},2
};

DRAM_ATTR const epd_init_2 Gdew0583z21::epd_panel_setting={
0x00,{
  0xCF,
  0x08},1
};

DRAM_ATTR const epd_init_3 Gdew0583z21::epd_boost={
0x06,{0xC7,0xCC,0x28},3
};

DRAM_ATTR const epd_init_4 Gdew0583z21::epd_resolution={
0x61,{
  0x02, //source 600
  0x58,
  0x01, //gate 448
  0xc0
},4};

// Constructor
Gdew0583z21::Gdew0583z21(EpdSpi& dio): 
  Adafruit_GFX(GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT),
  Epd(GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT), IO(dio)
{
  printf("Gdew0583z21() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT);  
}

//Initialize the display
void Gdew0583z21::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0583z21::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug
    IO.init(4, debug);
    fillScreen(EPD_WHITE);
}

void Gdew0583z21::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == EPD_WHITE);
  else if (color == EPD_BLACK) black = 0xFF;
  else if (color == EPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void Gdew0583z21::_wakeUp(){
  IO.reset(10);
  IO.cmd(epd_wakeup_power.cmd);
 
  for (int i=0;i<sizeof(epd_wakeup_power.data);++i) {
    IO.data(epd_wakeup_power.data[i]);
  }
  printf("Panel setting\n");
  IO.cmd(epd_panel_setting.cmd);
  IO.data(epd_panel_setting.data[0]); //0x0f KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f

  printf("PLL\n");
  IO.cmd(0x30);
  IO.data(0x3a);

  printf("VCOM\n");
  IO.cmd(0x82);
  IO.data(0x28);
  
  printf("Boost\n"); // Handles the intensity of the colors displayed
  IO.cmd(epd_boost.cmd);
  for (int i=0;i<sizeof(epd_boost.data);++i) {
    IO.data(epd_boost.data[i]);
  }


	IO.cmd(0X50);			//VCOM AND DATA INTERVAL SETTING
	IO.data(0x77);

	IO.cmd(0X60);			//TCON SETTING
	IO.data(0x22);

	IO.cmd(0X65);			//FLASH CONTROL
	IO.data(0x00);
  
  printf("Resolution setting\n");
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<sizeof(epd_resolution.data);++i) {
    IO.data(epd_resolution.data[i]);
  }
  printf("Flash mode\n");
  IO.cmd(0xe5);
	IO.data(0x03);

  // Power it on
  IO.cmd(0x04);
  _waitBusy("Power on");

}

void Gdew0583z21::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
  // IN GD example says bufferSize is 38880 (?)
  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n",sizeof(_buffer));
    
  for (uint32_t i = 0; i < sizeof(_buffer); ++i)
  {
    _send8pixel(_buffer[i], _red_buffer[i]);
    
    if (i%2000 == 0 && debug_enabled) {
       // Funny without outputting this to serial is not refreshing. Seems no need of rtc_wdt_feed();
       printf("%d ", (int)i);
       vTaskDelay(pdMS_TO_TICKS(10));   
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

void Gdew0583z21::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("Color epapers from Goodisplay do not support partial update. Full update triggered\n");
  update();
}

void Gdew0583z21::_waitBusy(const char* message){
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

void Gdew0583z21::_sleep(){
  // Flash sleep  
  IO.cmd(0x02);
  _waitBusy("Power Off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xa5);
}

void Gdew0583z21::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0583Z21_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0583Z21_WIDTH - x - w - 1;
      y = GDEW0583Z21_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0583Z21_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0583z21::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0583Z21_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0583Z21_WIDTH - x - 1;
      y = GDEW0583Z21_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0583Z21_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0583Z21_WIDTH / 8;

  // This formulas are from gxEPD that apparently got the color right:
  _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else if (color == EPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}

/**
 * There should be a smarter way to do this
 * if you find it, just make a PR
 * 0x04 RED
 * 0x03 WHITE 
 * 0x00 BLACK
 */
void Gdew0583z21::_send8pixel(uint8_t black, uint8_t red)
{ 
  // This loops 4 times, recollects 2 pixels, and sends them to IO.data (Is very slow like it is)
  for (uint8_t j = 0; j < 8; ++j)
  {
    uint8_t pix2 = (black & 0x80) ? 0x00 : (red & 0x80) ? 0x04 : 0x03; 
    pix2 <<= 4;
    black <<= 1;
    red <<= 1;
    j++;
    pix2 |= (black & 0x80) ? 0x00 : (red & 0x80) ? 0x04 : 0x03; 
    black <<= 1;
    red <<= 1;

    IO.dataBuffer(pix2);
  }

}

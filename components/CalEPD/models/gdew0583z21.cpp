#include "gdew0583z21.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// CMD, DATA, Databytes * Optional we are going to use sizeof(data)
DRAM_ATTR const epd_init_4 Gdew0583z21::epd_wakeup_power={
0x01,{
  0x07,
  0x07, // VGH=20V,VGL=-20V
  0x3F, // VDH=+15V
  0x3F, // VDH=-15V
  },4
};

DRAM_ATTR const epd_init_1 Gdew0583z21::epd_panel_setting={
0x00,{0x0F},1
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
    IO.init(4, true);
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

  IO.cmd(0x82);  // VCOM Voltage
  IO.data(0x28); // All temperature range
  
  IO.cmd(0xe5);  // Flash mode
  IO.data(0x03);
  // Power it on
  IO.cmd(0x04);
  _waitBusy("Power on");


  printf("Panel setting\n");
  IO.cmd(epd_panel_setting.cmd);
  IO.data(epd_panel_setting.data[0]); //0x0f KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f

  // Resolution setting
  printf("Resolution setting\n");
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<sizeof(epd_resolution.data);++i) {
    IO.data(epd_resolution.data[i]);
  }
  IO.cmd(0x15);
  IO.data(0x00);
  // Vcom and data interval settings
  IO.cmd(0x50);
  IO.data(0x11);
  IO.data(0x07);
  // TCON (???)
  IO.cmd(0x60);
  IO.data(0x22);
  
  // Withouth flash mode does not work (But is not on gxEPD)
  /* IO.cmd(0xe5);  // Flash mode
  IO.data(0x03);  */
}

void Gdew0583z21::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
  // IN GD example says bufferSize is 38880 (?)
  IO.cmd(0x10);
  printf("Sending a %d bytes black buffer via SPI\n",sizeof(_buffer));

  for (uint32_t i = 0; i < sizeof(_buffer); i++)
  {
    IO.dataBuffer((i < sizeof(_buffer)) ? ~_buffer[i] : 0xFF);
    
    if (i%2000==0) {
       rtc_wdt_feed();
       vTaskDelay(pdMS_TO_TICKS(10));
       if (debug_enabled) printf("%d ",i);
    }
  
  }
  printf("RED buffer\n");
  IO.cmd(0x13); // Red buffer
    for (uint32_t i = 0; i < sizeof(_red_buffer); i++)
  {
    IO.dataBuffer((i < sizeof(_red_buffer)) ? ~_red_buffer[i] : 0x00);
    if (i%2000==0) {
       rtc_wdt_feed();
       vTaskDelay(pdMS_TO_TICKS(10));
       if (debug_enabled) printf("%d ",i);
    }
  }
  IO.cmd(0x12);
  
  uint64_t endTime = esp_timer_get_time();
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);

  //vTaskDelay(pdMS_TO_TICKS(1000));
  _sleep();
}

uint16_t Gdew0583z21::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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

void Gdew0583z21::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: Not implemented\n");  
  vTaskDelay(GDEW0583Z21_PU_DELAY / portTICK_PERIOD_MS);
}

void Gdew0583z21::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (true){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
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

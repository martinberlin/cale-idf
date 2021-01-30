// Epaper: 5.65inch ACeP 7-Color  https://www.waveshare.com/product/displays/e-paper/5.65inch-e-paper-module-f.htm
// This epaper like most color models does not support partialUpdate
#include "color/wave5i7Color.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Wave5i7Color::Wave5i7Color(EpdSpi& dio): 
  Adafruit_GFX(WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT),
  Epd(WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT), IO(dio)
{
  printf("Wave5i7Color() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT);  
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
    fillScreen(EPD_WHITE);
}

void Wave5i7Color::fillScreen(uint16_t color)
{
  uint8_t pv = _color7(color);
  uint8_t pv2 = pv | pv << 4;
  for (uint32_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = pv2;
  }

  if (debug_enabled) printf("fillScreen(%x) black/red _buffer len:%d\n", color, sizeof(_buffer));
}

void Wave5i7Color::_wakeUp(){
  IO.reset(10);
  _waitBusy("epd_wakeup reset");  //waiting for the electronic paper IC to release the idle signal

  IO.cmd(0x01); //POWER SETTING
  IO.data(0x37); 
  IO.data(0x00);

  IO.cmd(0X00); //PANNEL SETTING
  IO.data(0xCF);
  IO.data(0x08);

  IO.cmd(0x06); //boost
  IO.data(0xc7);
  IO.data(0xcc);
  IO.data(0x28);

  IO.cmd(0x30);
  IO.data(0x3c); //PLL:    0-15:0x3C, 15+:0x3A

  IO.cmd(0X41); //TEMPERATURE SETTING
  IO.data(0x00);

  IO.cmd(0X50); //VCOM AND DATA INTERVAL SETTING
  IO.data(0x77);

  IO.cmd(0X60);  //TCON SETTING
  IO.data(0x22);

  IO.cmd(0x61);  // Resolution setting 600*448
  IO.data(0x02); //source 600
  IO.data(0x58);
  IO.data(0x01); //gate 448
  IO.data(0xc0);
  IO.cmd(0X82);  //VCOM VOLTAGE SETTING

  IO.data(0x28);    //all temperature  range
  IO.cmd(0xe5); //FLASH MODE
  IO.data(0x03);
}

void Wave5i7Color::update()
{
  printf("display.update() called\n");

  uint64_t startTime = esp_timer_get_time();
  _wakeUp();
  IO.cmd(0x04); // Power on
  _waitBusy("Power on");
  
  // Needed? Is on EPD2 but this epaper does not suport partial updates
  //IO.cmd(0x91); // Partial in
  //_setPartialRamArea(0, 0, WAVE5I7COLOR_WIDTH, WAVE5I7COLOR_HEIGHT);

  IO.cmd(0x10);

  // Send test buffer (WHITE)
  for (uint32_t i = 0; i < sizeof(_buffer); i++)
  {
    IO.data(_buffer[i]);
  }

  uint64_t endTime = esp_timer_get_time();

  IO.cmd(0x12);
  _waitBusy("0x12 display refresh");

  uint64_t powerOnTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  // DEBUG Disable sleep until Buffer is completely written and tested
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


  // Transpose partial window to 0,0 - TODO: Not sure if I need this (Check and clean)
  x -= _pw_x;
  y -= _pw_y;
  uint32_t i = x / 2 + uint32_t(y) * (_pw_w / 2);
  uint8_t pv = _color7(color);
      
  if (x & 1) _buffer[i] = (_buffer[i] & 0xF0) | pv;
    else _buffer[i] = (_buffer[i] & 0x0F) | (pv << 4);
}

/**
 * From GxEPD2 (Jean-Marc)
 * TODO: Should be migrated to Epd7Color base class so is not copied on other models
 * Converts from color constants to the right 4 bit pixel color in Acep epaper 7 color
 */
uint8_t Wave5i7Color::_color7(uint16_t color)
    {
      static uint16_t _prev_color = EPD_BLACK;
      static uint8_t _prev_color7 = 0x00; // black
      if (color == _prev_color) return _prev_color7;
      uint8_t cv7 = 0x00;
      switch (color)
      {
        case EPD_BLACK: cv7 = 0x00; break;
        case EPD_WHITE: cv7 = 0x01; break;
        case EPD_GREEN: cv7 = 0x02; break;
        case EPD_BLUE:  cv7 = 0x03; break;
        case EPD_RED:   cv7 = 0x04; break;
        case EPD_YELLOW: cv7 = 0x05; break;
        case EPD_ORANGE: cv7 = 0x06; break;
        default:
          {
            uint16_t red = color & 0xF800;
            uint16_t green = (color & 0x07E0) << 5;
            uint16_t blue = (color & 0x001F) << 11;
            if ((red < 0x8000) && (green < 0x8000) && (blue < 0x8000)) cv7 = 0x00; // black
            else if ((red >= 0x8000) && (green >= 0x8000) && (blue >= 0x8000)) cv7 = 0x01; // white
            else if ((red >= 0x8000) && (blue >= 0x8000)) cv7 = red > blue ? 0x04 : 0x03; // red, blue
            else if ((green >= 0x8000) && (blue >= 0x8000)) cv7 = green > blue ? 0x02 : 0x03; // green, blue
            else if ((red >= 0x8000) && (green >= 0x8000))
            {
              static const uint16_t y2o_lim = ((EPD_YELLOW - EPD_ORANGE) / 2 + (EPD_ORANGE & 0x07E0)) << 5;
              cv7 = green > y2o_lim ? 0x05 : 0x06; // yellow, orange
            }
            else if (red >= 0x8000) cv7 = 0x04; // red
            else if (green >= 0x8000) cv7 = 0x02; // green
            else cv7 = 0x03; // blue
          }
      }
      _prev_color = color;
      _prev_color7 = cv7;
      return cv7;
    }


void Wave5i7Color::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t ye = y + h - 1;
  x &= 0xFFF8; // byte boundary
  xe |= 0x0007; // byte boundary
  IO.cmd(0x90); // partial window
  IO.data(x / 256);
  IO.data(x % 256);
  IO.data(xe / 256);
  IO.data(xe % 256);
  IO.data(y / 256);
  IO.data(y % 256);
  IO.data(ye / 256);
  IO.data(ye % 256);
  IO.data(0x00); // distortion on right half
}

void Wave5i7Color::_send8pixel(uint8_t black_data, uint8_t color_data)
{
  for (uint8_t j = 0; j < 8; j++)
  {
    uint8_t t = 0x00; // black
    if (black_data & 0x80); // keep black
    else if (color_data & 0x80) t = 0x04; //color
    else t = 0x03; // white
    t <<= 4;
    black_data <<= 1;
    color_data <<= 1;
    j++;
    if (black_data & 0x80); // keep black
    else if (color_data & 0x80) t |= 0x04; //color
    else t |= 0x03; // white
    black_data <<= 1;
    color_data <<= 1;
    IO.data(t);
  }
}
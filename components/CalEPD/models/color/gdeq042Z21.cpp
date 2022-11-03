#include "color/gdeq042Z21.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Constructor
Gdeq042Z21::Gdeq042Z21(EpdSpi& dio): 
  Adafruit_GFX(GDEH042Z21_WIDTH, GDEH042Z21_HEIGHT),
  Epd(GDEH042Z21_WIDTH, GDEH042Z21_HEIGHT), IO(dio)
{
  printf("Gdeq042Z21() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEH042Z21_WIDTH, GDEH042Z21_HEIGHT);  
}

//Initialize the display
void Gdeq042Z21::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdeq042Z21::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    //setModel("GDEW042C37"); // Can be commented and set from your app_main()
    fillScreen(EPD_WHITE);
}

// Model setter. GDEH042Z15 is default but this class supports also GDEH042Z21  and  GDEW042C37
void Gdeq042Z21::setModel(std::string set_model) {
  model = set_model;
  printf("EPAPER model set to %s\n", set_model.c_str());
}

void Gdeq042Z21::_wakeUp(){
  IO.reset(10); 
  IO.cmd(0x04);  
  _waitBusy("epd_wakeup");; //waiting for the electronic paper IC to release the idle signal
    
  IO.cmd(0x00);  //panel setting
  IO.data(0x0f); // LUT from OTP 400x300
  
  IO.cmd(0x50);  //panel setting
  IO.data(0x77); // Extra setting sent by Freddie

  // Without this Black appeared washed out near RED. If colors appear washed out try to "use boost soft start"
  if (model=="Gdeq042Z21") {
    printf("Sending additional boost soft start settings for %s\n", model.c_str());
    IO.cmd(0x06);  // panel setting
    IO.data(0x17); // Boost soft start
    IO.data(0x18);
    IO.data(0x18);
  }
}

void Gdeq042Z21::_waitBusy(const char* message){
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

void Gdeq042Z21::_sleep(){
    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING  
    IO.data(0xf7);
       
    IO.cmd(0X02);   //power off
    _waitBusy("power off");
    IO.cmd(0X07);   //deep sleep
    IO.data(0xA5);
}

void Gdeq042Z21::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEH042Z21_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEH042Z21_WIDTH - x - w - 1;
      y = GDEH042Z21_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEH042Z21_HEIGHT - y - h - 1;
      break;
  }
}

void Gdeq042Z21::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();


  // BLACK: Write RAM for black(0)/white (1)
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEH042Z21_WIDTH/8;
  uint8_t x1buf[xLineBytes];

// Note that in IC specs is 0x10 BLACK and 0x13 RED
// BLACK: Write RAM
  IO.cmd(0x10);
  for(uint16_t y =  1; y <= GDEH042Z21_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : GDEH042Z21_8PIX_WHITE;
          x1buf[x-1] = data;
          if (x==xLineBytes) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }
   
   i = 0;

  // RED: Write RAM
  IO.cmd(0x13);
    for(uint16_t y =  1; y <= GDEH042Z21_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_red_buffer) ? _red_buffer[i] : GDEH042Z21_8PIX_RED_WHITE;
          //printf("%x ",data);
          x1buf[x-1] = data;
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

void Gdeq042Z21::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEH042Z21_WIDTH - x - 1;
      break;
    case 2:
      x = GDEH042Z21_WIDTH - x - 1;
      y = GDEH042Z21_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEH042Z21_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEH042Z21_WIDTH / 8;
  
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


void Gdeq042Z21::fillScreen(uint16_t color)
{
  // Fill screen will be inverted with the way is done NOW
  uint8_t black = GDEH042Z21_8PIX_WHITE;
  uint8_t red = GDEH042Z21_8PIX_RED_WHITE;
  if (color == EPD_WHITE) {
    if (debug_enabled) printf("fillScreen WHITE\n");
  } else if (color == EPD_BLACK) {
    black = GDEH042Z21_8PIX_BLACK;
    if (debug_enabled) printf("fillScreen BLACK SELECTED\n");
  } else if (color == EPD_RED) {
    red = GDEH042Z21_8PIX_RED;
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

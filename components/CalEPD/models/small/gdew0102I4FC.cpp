#include "small/gdew0102I4FC.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Constructor
Gdew0102I4FC::Gdew0102I4FC(EpdSpi& dio): 
  Adafruit_GFX(GDEW0102I4FC_WIDTH, GDEW0102I4FC_HEIGHT),
  Epd(GDEW0102I4FC_WIDTH, GDEW0102I4FC_HEIGHT), IO(dio)
{
  printf("Gdew0102I4FC() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0102I4FC_WIDTH, GDEW0102I4FC_HEIGHT);  
}

//Initialize the display
void Gdew0102I4FC::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0102I4FC::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    //setModel("GDEW042C37"); // Can be commented and set from your app_main()
    fillScreen(EPD_WHITE);
}


void Gdew0102I4FC::_wakeUp(){

  IO.reset(10);
    
  IO.cmd(0x00);  // Panel setting
  IO.data(0x5f); // OTP

  IO.cmd(0x2A);     
  IO.data(0x00); 
  IO.data(0x00);
  
  IO.cmd(0x04);  //Power on
  _waitBusy("epd_wakeup");; //waiting for the electronic paper IC to release the idle signal

  IO.cmd(0x50);  //Solve some black paper black border problems  
  IO.data(0x97);  
}

void Gdew0102I4FC::_waitBusy(const char* message){
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

void Gdew0102I4FC::_sleep(){
    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING  
    IO.data(0xf7);
       
    IO.cmd(0X02);   //power off
    _waitBusy("power off");
    IO.cmd(0X07);   //deep sleep
    IO.data(0xA5);
}

void Gdew0102I4FC::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0102I4FC_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0102I4FC_WIDTH - x - w - 1;
      y = GDEW0102I4FC_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0102I4FC_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0102I4FC::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();


  // BLACK: Write RAM for black(0)/white (1)
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW0102I4FC_WIDTH/8;
  uint8_t x1buf[xLineBytes];

// Note that in IC specs is 0x10 old data (?) and 0x13 new
  IO.cmd(0x10);
  uint8_t full_buff[GDEW0102I4FC_BUFFER_SIZE];
  for(uint16_t y =  0; y < GDEW0102I4FC_BUFFER_SIZE; y++) {
    full_buff[y] = 0xFF;
  }
  IO.data(full_buff, GDEW0102I4FC_BUFFER_SIZE);

// BLACK new data: Write RAM
  IO.cmd(0x13);
  for(uint16_t y =  1; y <= GDEW0102I4FC_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : GDEW0102I4FC_8PIX_WHITE;
          x1buf[x-1] = data;
          if (x==xLineBytes) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);     //DISPLAY REFRESH 

  _waitBusy("epaper refresh");
  uint64_t powerOnTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

void Gdew0102I4FC::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0102I4FC_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0102I4FC_WIDTH - x - 1;
      y = GDEW0102I4FC_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0102I4FC_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0102I4FC_WIDTH / 8;
  
  if (!color) {
    _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
}


void Gdew0102I4FC::fillScreen(uint16_t color)
{
  // Fill screen will be inverted with the way is done NOW
  uint8_t fill = GDEW0102I4FC_8PIX_WHITE;
  
  if (color == EPD_BLACK) {
    fill = GDEW0102I4FC_8PIX_BLACK;
  }
  
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = fill;
  }

  if (debug_enabled) printf("fillScreen(%x) black len:%d\n", fill, sizeof(_black_buffer));
}

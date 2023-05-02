#include "goodisplay/gdey042T81.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Constructor
Gdey042T81::Gdey042T81(EpdSpi& dio): 
  Adafruit_GFX(GDEY042T81_WIDTH, GDEY042T81_HEIGHT),
  Epd(GDEY042T81_WIDTH, GDEY042T81_HEIGHT), IO(dio)
{
  printf("Gdey042T81() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEY042T81_WIDTH, GDEY042T81_HEIGHT);  
}

//Initialize the display
void Gdey042T81::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdey042T81::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdey042T81::fillScreen(uint16_t color)
{
  uint8_t fill_color = GDEY042T81_8PIX_WHITE;
  if (color == EPD_BLACK) {
    fill_color = GDEY042T81_8PIX_BLACK;
  }
  
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = fill_color;
  }

  if (debug_enabled) printf("fillScreen(%x) black/red _buffer len:%d\n", color, sizeof(_black_buffer));
}

void Gdey042T81::_wakeUp(){
  if (fastmode == 0 && !is_powered) {
    IO.reset(10);
    _waitBusy("epd_wakeup reset");  //waiting for the electronic paper IC to release the idle signal
    is_powered = true;
    IO.cmd(0x12);     //SWRESET
    _waitBusy("epd_wakeup swreset");  //waiting for the electronic paper IC to release the idle signal
  }

  // This commands are similar to 042Z96 also SSD controller brand
  IO.cmd(0x21);
  IO.data(0x40);
  IO.data(0x00);

  IO.cmd(0x3C); // board
  IO.data(0x05);

  IO.cmd(0x1A); // Write to temperature register
  IO.data(0x5A);
  IO.cmd(0x22); // Load temperature value
  IO.data(0x91);
  IO.cmd(0x20);
  _waitBusy("epd_load temp");

  IO.cmd(0x11);  // Data entry mode
  IO.data(0x01);

  IO.cmd(0x44);
  IO.data(0x00); // RAM x address start at 0
  IO.data(0x31); // RAM x address end at 31h(49+1)*8->400
  IO.cmd(0x45);
  IO.data(0x2B);   // RAM y address start at 12Bh
  IO.data(0x01);
  IO.data(0x00); // RAM y address end at 00h
  IO.data(0x00);

  IO.cmd(0x4E);
  IO.data(0x00);
  IO.cmd(0x4F);
  IO.data(0x2B);
  IO.data(0x01);
}

void Gdey042T81::update()
{
  uint64_t startTime = esp_timer_get_time();
  _wakeUp();
  
  // BLACK: Write RAM for black(0)/white (1)
  IO.cmd(0x24);
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEY042T81_WIDTH/8;
  uint8_t x1buf[xLineBytes];
  // Curiosity doing it x++ is mirrored

    for(uint16_t y =  1; y <= GDEY042T81_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : GDEY042T81_8PIX_WHITE;
          x1buf[x-1] = data;
          if (x==xLineBytes) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }
  uint64_t endTime = esp_timer_get_time();
  
  IO.cmd(0x22);  //Display Update Control
  IO.data(0xC7);   
  IO.cmd(0x20);  //Activate Display Update Sequence
  _waitBusy("update");
  uint64_t refreshTime= esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp+send Buffer\n%llu refreshTime\n%llu total time in millis\n",
  (endTime-startTime)/1000, (refreshTime-endTime)/1000, (refreshTime-startTime)/1000);

  _sleep();
}

void Gdey042T81::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

// Public method
void Gdey042T81::deepsleep() {
  _sleep();
}

void Gdey042T81::_sleep() {
  if (!fastmode) {
    IO.cmd(0x10);
    IO.data(0x01);// power off
  }
  is_powered = false;
}

void Gdey042T81::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEY042T81_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEY042T81_WIDTH - x - w - 1;
      y = GDEY042T81_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEY042T81_HEIGHT - y - h - 1;
      break;
  }
}


void Gdey042T81::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  // MIRROR Issue. Swap X axis (For sure there is a smarter solution than this one)
  x = width()-x;
  // Check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEY042T81_WIDTH - x - 1;
      break;
    case 2:
      x = GDEY042T81_WIDTH - x - 1;
      y = GDEY042T81_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEY042T81_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEY042T81_WIDTH / 8;

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

  // This formulas are from gxEPD that apparently got the color right:
  _black_buffer[i] = (_black_buffer[i] & (GDEY042T81_8PIX_WHITE ^ (1 << (7 - x % 8))));

  if (color != EPD_WHITE) {
    _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  }
}

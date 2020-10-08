#include "gdeh042Z96.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Gdeh042Z96::Gdeh042Z96(EpdSpi& dio): 
  Adafruit_GFX(GDEH042Z96_WIDTH, GDEH042Z96_HEIGHT),
  Epd(GDEH042Z96_WIDTH, GDEH042Z96_HEIGHT), IO(dio)
{
  printf("Gdeh042Z96() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEH042Z96_WIDTH, GDEH042Z96_HEIGHT);  
}

//Initialize the display
void Gdeh042Z96::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdeh042Z96::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdeh042Z96::fillScreen(uint16_t color)
{
  uint8_t black = GDEH042Z96_8PIX_WHITE;
  uint8_t red = GDEH042Z96_8PIX_RED_WHITE;
  if (color == EPD_WHITE) {

  } else if (color == EPD_BLACK) {
    black = GDEH042Z96_8PIX_BLACK;
    printf("fillScreen BLACK SELECTED\n");
  } else if (color == EPD_RED) {
    red = GDEH042Z96_8PIX_RED;
    printf("fillScreen RED SELECTED\n");
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

  if (debug_enabled) printf("fillScreen(%x) black/red _buffer len:%d\n",color,sizeof(_black_buffer));
}

void Gdeh042Z96::_wakeUp(){
  IO.reset(10);
    _waitBusy("epd_wakeup reset");  //waiting for the electronic paper IC to release the idle signal
      IO.cmd(0x12);     //SWRESET
    _waitBusy("epd_wakeup swreset");  //waiting for the electronic paper IC to release the idle signal
  
    IO.cmd(0x74);
    IO.data(0x54);
    IO.cmd(0x7E);
    IO.data(0x3B);
    IO.cmd(0x2B);  // Reduce glitch under ACVCOM  
    IO.data(0x04);           
    IO.data(0x63);

    IO.cmd(0x0C);  // Soft start setting
    IO.data(0x8B);           
    IO.data(0x9C);
    IO.data(0x96);
    IO.data(0x0F);

    IO.cmd(0x01);  // Set MUX as 300
    IO.data(0x2B);           
    IO.data(0x01);
    IO.data(0x00);     

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
    IO.cmd(0x3C); // board
    IO.data(0x01); // HIZ

    IO.cmd(0x18);
    IO.data(0X80);
    IO.cmd(0x22);
    IO.data(0XB1);  //Load Temperature and waveform setting.
    IO.cmd(0x20);
    //waiting for the electronic paper IC to release the idle signal
    _waitBusy("epd_wakeup swreset");
    

    IO.cmd(0x4E); 
    IO.data(0x00);
    IO.cmd(0x4F); 
    IO.data(0x2B);
    IO.data(0x01);
}

void Gdeh042Z96::update()
{
  uint64_t startTime = esp_timer_get_time();
  _wakeUp();
  
  // BLACK: Write RAM for black(0)/white (1)
  IO.cmd(0x24);
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEH042Z96_WIDTH/8;
  uint8_t x1buf[xLineBytes];
  // Curiosity doing it x++ is mirrored

    for(uint16_t y =  1; y <= GDEH042Z96_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : GDEH042Z96_8PIX_WHITE;
          x1buf[x-1] = data;
          if (x==xLineBytes) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }
  
  // RED: Write RAM for red(1)/white (0)
  i = 0;
  IO.cmd(0x26);
    for(uint16_t y =  1; y <= GDEH042Z96_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_red_buffer) ? _red_buffer[i] : GDEH042Z96_8PIX_RED_WHITE;
          x1buf[x-1] = data;
          if (x==xLineBytes) {
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
  uint64_t powerOnTime = esp_timer_get_time();

  // GxEPD comment: Avoid double full refresh after deep sleep wakeup
  // if (_initial) {  // --> Original deprecated 
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

void Gdeh042Z96::_waitBusy(const char* message){
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

void Gdeh042Z96::_sleep(){
  
  IO.cmd(0x10);
  IO.data(0x01);// power off
}

void Gdeh042Z96::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEH042Z96_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEH042Z96_WIDTH - x - w - 1;
      y = GDEH042Z96_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEH042Z96_HEIGHT - y - h - 1;
      break;
  }
}


void Gdeh042Z96::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  // MIRROR Issue. Swap X axis (For sure there is a smarter solution than this one)
  x = width()-x;
  // Check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEH042Z96_WIDTH - x - 1;
      break;
    case 2:
      x = GDEH042Z96_WIDTH - x - 1;
      y = GDEH042Z96_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEH042Z96_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEH042Z96_WIDTH / 8;

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
  _black_buffer[i] = (_black_buffer[i] & (GDEH042Z96_8PIX_WHITE ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (GDEH042Z96_8PIX_RED ^ (1 << (7 - x % 8)))); // white

  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  else if (color == EPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));

}

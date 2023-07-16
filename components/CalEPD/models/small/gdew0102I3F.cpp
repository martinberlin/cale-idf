#include "small/gdew0102I3F.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Constructor
Gdew0102I3F::Gdew0102I3F(EpdSpi& dio): 
  Adafruit_GFX(GDEW0102I3F_WIDTH, GDEW0102I3F_HEIGHT),
  Epd(GDEW0102I3F_WIDTH, GDEW0102I3F_HEIGHT), IO(dio)
{
  printf("Gdew0102I3F() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0102I3F_WIDTH, GDEW0102I3F_HEIGHT);  
}

//Initialize the display
void Gdew0102I3F::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0102I3F::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    //setModel("GDEW042C37"); // Can be commented and set from your app_main()
    fillScreen(EPD_WHITE);
}


void Gdew0102I3F::_wakeUp(){

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

void Gdew0102I3F::_wakeUpPart(){
  _using_partial_mode = true;
  IO.reset(10);
  
  IO.cmd(0x00);  // Panel setting
  IO.data(0x6F); // LUT from Registers

  IO.cmd(0x30);  // PLL
  IO.data(0x05); // 15Hz
  IO.cmd(0x50);  // VCOM and Data interval setting
  IO.data(0xF2); // DDX 11 : differential, VBD 11 : vcom, CDI 2 : 5 hsync
  IO.cmd(0x82);  // Vcom DC Setting
  IO.data(0x00);    // -0.1 V
  
  IO.cmd(0x23);
  IO.data(lut_w_partial, sizeof(lut_w_partial));
  IO.cmd(0x24);
  IO.data(lut_b_partial, sizeof(lut_b_partial));

  IO.cmd(0x04);  //Power on
  _waitBusy("epd_wakeup");; //waiting for the electronic paper IC to release the idle signal
}

void Gdew0102I3F::_waitBusy(const char* message){
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

void Gdew0102I3F::_sleep(){
    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING  
    IO.data(0xf7);
       
    IO.cmd(0X02);   //power off
    _waitBusy("power off");
    IO.cmd(0X07);   //deep sleep
    IO.data(0xA5);
}

void Gdew0102I3F::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0102I3F_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0102I3F_WIDTH - x - w - 1;
      y = GDEW0102I3F_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0102I3F_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0102I3F::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();


  // BLACK: Write RAM for black(0)/white (1)
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW0102I3F_WIDTH/8;
  uint8_t x1buf[xLineBytes];

// Note that in IC specs is 0x10 old data (?) and 0x13 new
  IO.cmd(0x10);
  uint8_t full_buff[GDEW0102I3F_BUFFER_SIZE];
  for(uint16_t y =  0; y < GDEW0102I3F_BUFFER_SIZE; y++) {
    full_buff[y] = 0xFF;
  }
  IO.data(full_buff, GDEW0102I3F_BUFFER_SIZE);

// BLACK new data: Write RAM
  IO.cmd(0x13);
  for(uint16_t y =  1; y <= GDEW0102I3F_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_black_buffer) ? _black_buffer[i] : GDEW0102I3F_8PIX_WHITE;
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

void Gdew0102I3F::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0102I3F_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0102I3F_WIDTH - x - 1;
      y = GDEW0102I3F_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0102I3F_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0102I3F_WIDTH / 8;
  
  if (!color) {
    _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
    }
}


void Gdew0102I3F::fillScreen(uint16_t color)
{
  // Fill screen will be inverted with the way is done NOW
  uint8_t fill = GDEW0102I3F_8PIX_WHITE;
  
  if (color == EPD_BLACK) {
    fill = GDEW0102I3F_8PIX_BLACK;
  }
  
  for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
  {
    _black_buffer[x] = fill;
  }

  if (debug_enabled) printf("fillScreen(%x) black len:%d\n", fill, sizeof(_black_buffer));
}

void Gdew0102I3F::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  uint16_t w = (x + xe - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t h = y + ye - 1;
  x &= 0xFFF8;
  IO.cmd(0x90); // partial window
  IO.data(x);
  IO.data(w);
  IO.data(y);
  IO.data(h);
  IO.data(0x00);
}

void Gdew0102I3F::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation) {
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEW0102I3F_WIDTH) {
    ESP_LOGE(TAG, "Given width exceeds display");
    return;
  }
  if (y >= GDEW0102I3F_HEIGHT) {
    ESP_LOGE(TAG, "Given height exceeds display");
    return;
  }
  if (!_using_partial_mode) {
    _wakeUpPart();
    printf("updateWindow is still not implemented 100%% correctly\n");
  }
  uint16_t xe = gx_uint16_min(GDEW0102I3F_WIDTH, x + w) ;
  uint16_t ye = gx_uint16_min(GDEW0102I3F_HEIGHT, y + h) ;
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  uint8_t w1 = w;
  w1 += x % 8;
  if (w1 % 8 > 0) w1 += 8 - w1 % 8;
  x -= x % 8;

  // This command makes the display enter partial mode
  IO.cmd(0x91); // partial in
  // Here it sets where in RAM is going to write it
  _setPartialRamArea(x, y, w1, h);
  // NO need to fill 0x10 buffer or send it for partial 
  /* IO.cmd(0x10);
  uint8_t full_buff[GDEW0102I3F_BUFFER_SIZE];
  for(uint16_t y =  0; y < GDEW0102I3F_BUFFER_SIZE; y++) {
    full_buff[y] = 0xFF;
  }
  IO.data(full_buff, GDEW0102I3F_BUFFER_SIZE); */
  // New data
  IO.cmd(0x13);
  uint8_t temp_buffer[ye+1*xe_bx];
  uint16_t buff_pointer = 0;
  for (int16_t y1 = y; y1 <= ye+1; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GDEW0102I3F_WIDTH/ 8) + x1;
      temp_buffer[buff_pointer] = _black_buffer[idx]; // white is 0xFF on device
      buff_pointer++;
    }
  }
  IO.data(temp_buffer, buff_pointer);

  IO.cmd(0x12); // Refresh
  _waitBusy("partial");
  IO.cmd(0x92); // Partial out
}
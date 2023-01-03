#include "gdew042t2.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
/*
 The EPD needs a bunch of command/data values to be initialized. They are send using the IO class
*/

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//full screen update LUT
DRAM_ATTR const epd_init_44 Gdew042t2::lut_vcom0_full={
0x20, {
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},44};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_ww_full={
0x21, {
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_bw_full={
0x22,{
  0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_wb_full={
0x23,{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_bb_full={
0x24,{
  0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
  0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
  0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

// new waveform created by Jean-Marc Zingg for the actual panel
#define T1 25 // color change charge balance pre-phase
#define T2  1 // color change or sustain charge balance pre-phase
#define T3  2 // color change or sustain phase
#define T4 25 // color change phase

DRAM_ATTR const epd_init_44 Gdew042t2::lut_20_vcom0_partial={
0x20,{
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00
},44};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_21_ww_partial={
0x21,{
  0x18, T1, T2, T3, T4, 1, // 00 01 10 00
  0x00,  1,  0,  0,  0, 1, // gnd phase - 12 till here
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_22_bw_partial={
0x22,{ // 10 w
  0x5A, T1, T2, T3, T4, 1, // 01 01 10 10
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_23_wb_partial={
0x23,{
  0xA5, T1, T2, T3, T4, 1, // 10 10 01 01
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2::lut_24_bb_partial={
0x24,{
  0x24, T1, T2, T3, T4, 1, // 00 10 01 00
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_power_4 Gdew042t2::epd_wakeup_power={
0x01,{0x03,0x00,0x2b,0x2b},4
};

DRAM_ATTR const epd_init_3 Gdew042t2::epd_soft_start={
0x06,{0x17,0x17,0x17},3
};

DRAM_ATTR const epd_init_1 Gdew042t2::epd_panel_setting={
0x00,{0x3f},1
};

DRAM_ATTR const epd_init_1 Gdew042t2::epd_pll={
0x30,{0x3a},1
};

DRAM_ATTR const epd_init_4 Gdew042t2::epd_resolution={
0x61,{GDEW042T2_WIDTH/256,
GDEW042T2_WIDTH%256,
GDEW042T2_HEIGHT/256,
GDEW042T2_HEIGHT%256
},4};

// Constructor
Gdew042t2::Gdew042t2(EpdSpi& dio): 
  Adafruit_GFX(GDEW042T2_WIDTH, GDEW042T2_HEIGHT),
  Epd(GDEW042T2_WIDTH, GDEW042T2_HEIGHT), IO(dio)
{
  printf("Gdew042t2() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW042T2_WIDTH, GDEW042T2_HEIGHT);  
}

void Gdew042t2::initFullUpdate(){
    IO.cmd(0x00);   //300x400 B/W mode, LUT set by register
    IO.data(0x3F);


    IO.cmd(lut_vcom0_full.cmd);
    IO.data(lut_vcom0_full.data,lut_vcom0_full.databytes);
   
    IO.cmd(lut_ww_full.cmd);
    IO.data(lut_ww_full.data,lut_ww_full.databytes);

    IO.cmd(lut_bw_full.cmd);
    IO.data(lut_bw_full.data,lut_bw_full.databytes);

    IO.cmd(lut_wb_full.cmd);
    IO.data(lut_wb_full.data,lut_wb_full.databytes);

    IO.cmd(lut_bb_full.cmd);
    IO.data(lut_bb_full.data,lut_bb_full.databytes);
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

void Gdew042t2::initPartialUpdate(){
  IO.cmd(0x00);
  IO.data(0x3F); //300x400 B/W mode, LUT set by register

  // LUT Tables for partial update. Send them directly in 42 bytes chunks. In total 210 bytes
  IO.cmd(lut_20_vcom0_partial.cmd);
  IO.data(lut_20_vcom0_partial.data,lut_20_vcom0_partial.databytes);


  IO.cmd(lut_21_ww_partial.cmd);
  IO.data(lut_21_ww_partial.data,lut_21_ww_partial.databytes);

  IO.cmd(lut_22_bw_partial.cmd);
  IO.data(lut_22_bw_partial.data,lut_22_bw_partial.databytes);

  IO.cmd(lut_23_wb_partial.cmd);
  IO.data(lut_23_wb_partial.data,lut_23_wb_partial.databytes);

  IO.cmd(lut_24_bb_partial.cmd);
  IO.data(lut_24_bb_partial.data,lut_24_bb_partial.databytes);
 }

//Initialize the display
void Gdew042t2::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew042t2::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdew042t2::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_BLACK) ? GDEW042T2_8PIX_BLACK : GDEW042T2_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

void Gdew042t2::_wakeUp(){
  IO.reset(10);
//IMPORTANT: Some EPD controllers like to receive data byte per byte
//So this won't work:
//IO.data(epd_wakeup_power.data,epd_wakeup_power.databytes);
  

  IO.cmd(epd_wakeup_power.cmd);
  for (int i=0;i<epd_wakeup_power.databytes;++i) {
    IO.data(epd_wakeup_power.data[i]);
  }
 
  IO.cmd(epd_soft_start.cmd);
  for (int i=0;i<epd_soft_start.databytes;++i) {
    IO.data(epd_soft_start.data[i]);
  }
  
  IO.cmd(epd_panel_setting.cmd);
  for (int i=0;i<epd_panel_setting.databytes;++i) {
    IO.data(epd_panel_setting.data[i]);
  }
  
  IO.cmd(epd_pll.cmd);
  for (int i=0;i<epd_pll.databytes;++i) {
    IO.data(epd_pll.data[i]);
  }
  //resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<epd_resolution.databytes;++i) {
    IO.data(epd_resolution.data[i]);
  }

  IO.cmd(0x82); // vcom_DC setting
  IO.data(0x12);   // -0.1 + 18 * -0.05 = -1.0V from OTP, slightly better
  IO.cmd(0x50); // VCOM AND DATA INTERVAL SETTING
  IO.data(0xd7);    // border floating to avoid flashing
  IO.cmd(0x04);

  _waitBusy("epd_wakeup_power");
  initFullUpdate();
}

void Gdew042t2::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x13);
  // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  uint16_t i = 0;
  uint8_t xLineBytes = GDEW042T2_WIDTH/8;
  uint8_t x1buf[xLineBytes];
    for(uint16_t y =  1; y <= GDEW042T2_HEIGHT; y++) {
        for(uint16_t x = 1; x <= xLineBytes; x++) {
          uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x00;
          x1buf[x-1] = data;
          if (x==xLineBytes) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
          ++i;
        }
    }

  // v1 way to do it (Byte per byte toogling CS pin low->high)
  // Check 0.9.2 version: https://github.com/martinberlin/CalEPD/blob/0.9.2/models/gdew042t2.cpp#L278
  
  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);
  _waitBusy("update");
  uint64_t powerOnTime = esp_timer_get_time();

  // GxEPD comment: Avoid double full refresh after deep sleep wakeup
  // if (_initial) {  // --> Original deprecated 
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

uint16_t Gdew042t2::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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
  //IO.data(0x01);         // Not any visual difference
  IO.data(0x00);           // Not any visual difference
  return (7 + xe - x) / 8; // number of bytes to transfer per line
}

void Gdew042t2::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow is still being tested\n\n");
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEW042T2_WIDTH) return;
  if (y >= GDEW042T2_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEW042T2_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEW042T2_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  // _wakeUp has to be done always, since after update() it goes to sleep
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  initPartialUpdate();

  IO.cmd(0x91); // partial in
  _setPartialRamArea(x, y, xe, ye);
  IO.cmd(0x13);
  
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GDEW042T2_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
      IO.data(data); // white is 0xFF on device
    }
  }

  IO.cmd(0x92);      // partial out
  IO.cmd(0x12);      // display refresh
  _waitBusy("updateWindow");

  IO.cmd(0x91);      // partial out
  _setPartialRamArea(x, y, xe, ye);
  IO.cmd(0x13);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
    {
      uint16_t idx = y1 * (GDEW042T2_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(data);
    }
  }
  IO.cmd(0x92); // partial out
}

void Gdew042t2::_waitBusy(const char* message){
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

void Gdew042t2::_sleep(){
  IO.cmd(0x50); // border floating
  IO.cmd(0x17);
  IO.data(0x02);// power off
  _waitBusy("power_off");
  IO.cmd(0x07);
  IO.data(0xA5);// power off
}

void Gdew042t2::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW042T2_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW042T2_WIDTH - x - w - 1;
      y = GDEW042T2_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW042T2_HEIGHT - y - h - 1;
      break;
  }
}


void Gdew042t2::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW042T2_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW042T2_WIDTH - x - 1;
      y = GDEW042T2_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW042T2_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW042T2_WIDTH / 8;

  if (color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
}

#include "gdew0213i5f.h"
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
DRAM_ATTR const epd_init_44 Gdew0213i5f::lut_20_vcomDC={
0x20, {
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
},44};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_21_ww={
0x21, {
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_22_bw={
0x22,{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_23_wb={
0x23,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_24_bb={
0x24,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_5 Gdew0213i5f::epd_wakeup_power={
0x01,{0x03,0x00,0x2b,0x2b,0x03},5
};

DRAM_ATTR const epd_init_3 Gdew0213i5f::epd_soft_start={
0x06,{0x17,0x17,0x17},3
};

DRAM_ATTR const epd_init_2 Gdew0213i5f::epd_panel_setting={
0x00,{0xbf,0x0d},2
};

DRAM_ATTR const epd_init_1 Gdew0213i5f::epd_pll={
0x30,{0x3a},1
};

DRAM_ATTR const epd_init_3 Gdew0213i5f::epd_resolution={
0x61,{GDEW0213I5F_WIDTH,
GDEW0213I5F_HEIGHT >> 8,
GDEW0213I5F_HEIGHT & 0xFF
},3};

//partial screen update LUT
//#define Tx19 0x19 // original value is 25 (phase length)
#define Tx19 0x28   // new value for test is 40 (phase length)

DRAM_ATTR const epd_init_44 Gdew0213i5f::lut_20_vcomDC_partial={
0x20, {
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
},44};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_21_ww_partial={
0x21, {
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_22_bw_partial={
0x22, {
  0x80, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_23_wb_partial={
0x23, {
  0x40, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR const epd_init_42 Gdew0213i5f::lut_24_bb_partial={
0x24, {
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

// Partial Update Delay, may have an influence on degradation
#define GDEW0213I5F_PU_DELAY 100


// Constructor
Gdew0213i5f::Gdew0213i5f(EpdSpi& dio): 
  Adafruit_GFX(GDEW0213I5F_WIDTH, GDEW0213I5F_HEIGHT),
  Epd(GDEW0213I5F_WIDTH, GDEW0213I5F_HEIGHT), IO(dio)
{
  printf("Gdew0213i5f() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0213I5F_WIDTH, GDEW0213I5F_HEIGHT);  
}

void Gdew0213i5f::initFullUpdate(){
    IO.cmd(0x82);  //vcom_DC setting
    IO.data(0x08);
    
    // Works also commenting this two. Has default? (Check specs)
    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(0x97); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57

    // Every next cmd/data seems to be essential for initialization
    IO.cmd(lut_20_vcomDC.cmd);
    IO.data(lut_20_vcomDC.data,lut_20_vcomDC.databytes);
   
    IO.cmd(lut_21_ww.cmd);
    IO.data(lut_21_ww.data,lut_21_ww.databytes);

    IO.cmd(lut_22_bw.cmd);
    IO.data(lut_22_bw.data,lut_22_bw.databytes);

    IO.cmd(lut_23_wb.cmd);
    IO.data(lut_23_wb.data,lut_23_wb.databytes);

    IO.cmd(lut_24_bb.cmd);
    IO.data(lut_24_bb.data,lut_24_bb.databytes);
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

void Gdew0213i5f::initPartialUpdate(){
    IO.cmd(0x82);  //vcom_DC setting
    IO.data(0x08);

    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(0x17);

    IO.cmd(lut_20_vcomDC_partial.cmd);
    IO.data(lut_20_vcomDC_partial.data,lut_20_vcomDC_partial.databytes);
   
    IO.cmd(lut_21_ww_partial.cmd);
    IO.data(lut_21_ww_partial.data,lut_21_ww_partial.databytes);

    IO.cmd(lut_22_bw_partial.cmd);
    IO.data(lut_22_bw_partial.data,lut_22_bw_partial.databytes);

    IO.cmd(lut_23_wb_partial.cmd);
    IO.data(lut_23_wb_partial.data,lut_23_wb_partial.databytes);

    IO.cmd(lut_24_bb_partial.cmd);
    IO.data(lut_24_bb_partial.data,lut_24_bb_partial.databytes);
    if (debug_enabled) printf("initPartialUpdate() LUT\n");
}

//Initialize the display
void Gdew0213i5f::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0213i5f::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdew0213i5f::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_WHITE) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

void Gdew0213i5f::_wakeUp(){
  IO.reset(20);

  IO.cmd(epd_wakeup_power.cmd);
  IO.data(epd_wakeup_power.data,5);
  
  IO.cmd(epd_soft_start.cmd);
  IO.data(epd_soft_start.data,3);
  IO.cmd(0x04);
  _waitBusy("epd_wakeup_power");

  // [1] LUT from register, 128x296
  // [2] VCOM to 0V fast
  IO.cmd(epd_panel_setting.cmd);
  IO.data(epd_panel_setting.data,2);

  // 3a 100HZ   29 150Hz 39 200HZ 31 171HZ
  IO.cmd(epd_pll.cmd);
  IO.data(epd_pll.data,1);   

  //resolution setting
  IO.cmd(epd_resolution.cmd);
  IO.data(epd_resolution.data,3);

  initFullUpdate();
}

void Gdew0213i5f::update()
{
  _using_partial_mode = false;
  _wakeUp();

  IO.cmd(0x10);

  // In EPD here it wrote the full buffer with 0xFF. Note doing it like this is not refreshing. Todo: Check epaper tech specs
  /* uint8_t _wbuffer[GDEW0213I5F_BUFFER_SIZE];
    for (uint16_t x = 0; x < GDEW0213I5F_BUFFER_SIZE; x++){
    _wbuffer[x] = 0xFF;
  }
  IO.data(_wbuffer, sizeof(_wbuffer)); */
  
  IO.cmd(0x13);

  IO.data(_buffer,sizeof(_buffer));

  IO.cmd(0x12);
  _waitBusy("update");
  _sleep();
}

uint16_t Gdew0213i5f::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8; // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.cmd(0x90); // partial window
  IO.data(x % 256);
  IO.data(xe % 256);
  IO.data(y / 256);
  IO.data(y % 256);
  IO.data(ye / 256);
  IO.data(ye % 256);
  IO.data(0x01);
  IO.data(0x00);
  return (7 + xe - x) / 8; // number of bytes to transfer per line
}

void Gdew0213i5f::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("deprecated: updateWindow does not work\n");
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEW0213I5F_WIDTH) return;
  if (y >= GDEW0213I5F_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEW0213I5F_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEW0213I5F_HEIGHT, y + h) - 1;
  // x &= 0xFFF8; // byte boundary, not needed here
  uint16_t xs_bx = x / 8;
  uint16_t xe_bx = (xe + 7) / 8;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  initPartialUpdate();
  for (uint16_t twice = 0; twice < 2; twice++)
  { // leave both controller buffers equal
    IO.cmd(0x91); // partial in
    _setPartialRamArea(x, y, xe, ye);
    IO.cmd(0x13);

    uint16_t counter = 0;
    uint8_t data[GDEW0213I5F_WIDTH*GDEW0213I5F_HEIGHT];
    for (int16_t y1 = y; y1 <= ye; y1++)
    {
      for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
      {
        uint16_t idx = y1 * (GDEW0213I5F_WIDTH / 8) + x1;
        data[counter] = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        //uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
        //IO.data(~data); // white is 0xFF on device
        ++counter;
      }
    }
    IO.data(data, counter);
    IO.cmd(0x12);      // display refresh
    _waitBusy("updateWindow");
    IO.cmd(0x92);      // partial out
  } // leave both controller buffers equal
  vTaskDelay(GDEW0213I5F_PU_DELAY);
}

void Gdew0213i5f::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("deprecated: updateToWindow does not work\n");
  if (using_rotation)
  {
    switch (getRotation())
    {
      case 1:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        xs = GDEW0213I5F_WIDTH - xs - w - 1;
        xd = GDEW0213I5F_WIDTH - xd - w - 1;
        break;
      case 2:
        xs = GDEW0213I5F_WIDTH - xs - w - 1;
        ys = GDEW0213I5F_HEIGHT - ys - h - 1;
        xd = GDEW0213I5F_WIDTH - xd - w - 1;
        yd = GDEW0213I5F_HEIGHT - yd - h - 1;
        break;
      case 3:
        swap(xs, ys);
        swap(xd, yd);
        swap(w, h);
        ys = GDEW0213I5F_HEIGHT - ys  - h - 1;
        yd = GDEW0213I5F_HEIGHT - yd  - h - 1;
        break;
    }
  }
  if (xs >= GDEW0213I5F_WIDTH) return;
  if (ys >= GDEW0213I5F_HEIGHT) return;
  if (xd >= GDEW0213I5F_WIDTH) return;
  if (yd >= GDEW0213I5F_HEIGHT) return;
  // the screen limits are the hard limits
  uint16_t xde = gx_uint16_min(GDEW0213I5F_WIDTH, xd + w) - 1;
  uint16_t yde = gx_uint16_min(GDEW0213I5F_HEIGHT, yd + h) - 1;
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  initPartialUpdate();

  for (uint16_t twice = 0; twice < 2; twice++)
  { // leave both controller buffers equal
    IO.cmd(0x91); // partial in
    // soft limits, must send as many bytes as set by _SetRamArea
    uint16_t yse = ys + yde - yd;
    uint16_t xss_d8 = xs / 8;
    uint16_t xse_d8 = xss_d8 + _setPartialRamArea(xd, yd, xde, yde);
    IO.cmd(0x13);
    uint16_t counter = 0;
    //uint8_t data[GDEW0213I5F_WIDTH*GDEW0213I5F_HEIGHT];
    for (int16_t y1 = ys; y1 <= yse; y1++)
    {
      for (int16_t x1 = xss_d8; x1 < xse_d8; x1++)
      {
        uint16_t idx = y1 * (GDEW0213I5F_WIDTH / 8) + x1;
        uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00; // white is 0x00 in buffer
        IO.data(data);
        counter++;
      }
    }
    //IO.data(data, counter); // white is 0xFF on device

    IO.cmd(0x12);      //display refresh
    _waitBusy("updateToWindow");
    IO.cmd(0x92); // partial out
  } // leave both controller buffers equal
  vTaskDelay(GDEW0213I5F_PU_DELAY); 
}

void Gdew0213i5f::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>1800000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew0213i5f::_sleep(){
  IO.cmd(0x02); // power off display
  _waitBusy("power_off");
  IO.cmd(0x07); // deep sleep
  IO.data(0xa5);
}

void Gdew0213i5f::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0213I5F_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0213I5F_WIDTH - x - w - 1;
      y = GDEW0213I5F_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0213I5F_HEIGHT - y - h - 1;
      break;
  }
}


void Gdew0213i5f::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0213I5F_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0213I5F_WIDTH - x - 1;
      y = GDEW0213I5F_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0213I5F_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0213I5F_WIDTH / 8;

  if (color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

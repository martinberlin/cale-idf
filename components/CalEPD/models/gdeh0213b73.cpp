#include "gdeh0213b73.h"
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
#define GDEH0213B73_PU_DELAY 300
//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//full screen update LUT
DRAM_ATTR const epd_lut_100 Gdeh0213b73::lut_data_full={
0x32, {
  0xA0,  0x90, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x50, 0x90, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xA0, 0x90, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x50, 0x90, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x0F, 0x0F, 0x00, 0x00, 0x00,
  0x0F, 0x0F, 0x00, 0x00, 0x03,
  0x0F, 0x0F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00
},100};

DRAM_ATTR const epd_lut_100 Gdeh0213b73::lut_data_part={
0x32, {
  0x40,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0x0A, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
},100};

// Constructor GDEH0213B73
Gdeh0213b73::Gdeh0213b73(EpdSpi& dio): 
  Adafruit_GFX(GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT),
  Epd(GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT), IO(dio)
{
  printf("Gdeh0213b73() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT);  
}

void Gdeh0213b73::initFullUpdate(){
    _wakeUp();

    cmd(lut_data_full.cmd);
    for (uint16_t i = 0; i < sizeof(lut_data_full.data); i++) {
      IO.data(lut_data_full.data[i]);
    }

    _powerOn();
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

void Gdeh0213b73::initPartialUpdate(){
  _wakeUp();

  cmd(0x2C);      // VCOM Voltage
  IO.data(0x26);  // ???

  // Send partial update LUT table 0x32 -> LUT data
  cmd(lut_data_part.cmd);
  for (uint16_t i = 0; i < sizeof(lut_data_part.data); i++) {
    IO.data(lut_data_part.data[i]);
  }

  _powerOn();
  if (debug_enabled) printf("initPartialUpdate() LUT\n");
}

//Initialize the display
void Gdeh0213b73::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdeh0213b73::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdeh0213b73::fillScreen(uint16_t color)
{
  uint8_t data = (color == EPD_WHITE) ? 0x00 : 0xFF;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}


void Gdeh0213b73::update()
{
  _using_partial_mode = false;
  initFullUpdate();
  cmd(0x24); 
  
  for (uint16_t y = 0; y < GDEH0213B73_HEIGHT; y++) {
    for (uint16_t x = 0; x < GDEH0213B73_WIDTH / 8; x++)
    {
      uint16_t idx = y * (GDEH0213B73_WIDTH / 8) + x;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  cmd(0x26);
  for (uint16_t y = 0; y < GDEH0213B73_HEIGHT; y++) {
    for (uint16_t x = 0; x < GDEH0213B73_WIDTH / 8; x++)
    {
      uint16_t idx = y * (GDEH0213B73_WIDTH / 8) + x;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  
  // Update full display - next 3 lines
  cmd(0x22);
  IO.data(0xc7);
  cmd(0x20);
  _waitBusy("update full");

  _sleep(); // power off
}

uint16_t Gdeh0213b73::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  // Not implemented
  printf("Method not implemented\n");
  return 0;
}

void Gdeh0213b73::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEH0213B73_WIDTH) return;
  if (y >= GDEH0213B73_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEH0213B73_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEH0213B73_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8;
  initPartialUpdate();
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  _waitBusy("updateWindow I");
  cmd(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEH0213B73_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(data);
    }
  }
  //_Update_Part();
  cmd(0x22);
  IO.data(0x04); // use Mode 1 for GxEPD
  cmd(0x20);
  _waitBusy("updateWindow II");
  vTaskDelay(GDEH0213B73_PU_DELAY / portTICK_PERIOD_MS);

  // update erase buffer
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  _waitBusy("updateWindow III erase buffer");
  cmd(0x26);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEH0213B73_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(data);
    }
  }
  vTaskDelay(GDEH0213B73_PU_DELAY / portTICK_PERIOD_MS);
  
}

void Gdeh0213b73::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("Method not implemented\n");
}

void Gdeh0213b73::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>1000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdeh0213b73::cmd(uint8_t command){
  char buffer[3];
  sprintf(buffer,"%x",command);
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
    _waitBusy(buffer);
  }
  IO.cmd(command);
}

void Gdeh0213b73::_sleep(){
  cmd(0x22); // power off display
  IO.data(0xc0);
  cmd(0x20);
  _waitBusy("power_off");
}

void Gdeh0213b73::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
   switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEH0213B73_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEH0213B73_WIDTH - x - w - 1;
      y = GDEH0213B73_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEH0213B73_HEIGHT - y - h - 1;
      break;
  }
}


void Gdeh0213b73::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEH0213B73_VISIBLE_WIDTH - x - 1;
      break;
    case 2:
      x = GDEH0213B73_VISIBLE_WIDTH - x - 1;
      y = GDEH0213B73_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEH0213B73_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEH0213B73_WIDTH / 8;
  if (!color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
}

// _InitDisplay generalizing names here
void Gdeh0213b73::_wakeUp(){
  cmd(0x74); //set analog block control
  IO.data(0x54);
  cmd(0x7E); //set digital block control
  IO.data(0x3B);

  cmd(0x01); //Driver output control
  IO.data(0xF9);
  IO.data(0x00);
  IO.data(0x00);
  cmd(0x11); //data entry mode
  IO.data(0x01);
  cmd(0x44); //set Ram-X address start/end position
  IO.data(0x00);
  IO.data(0x0F);//0x0C-->(15+1)*8=128
  cmd(0x45); //set Ram-Y address start/end position
  IO.data(0xF9);//0xF9-->(249+1)=250
  IO.data(0x00);
  IO.data(0x00);
  IO.data(0x00);

  cmd(0x3C); //BorderWavefrom 
  IO.data(0x03);

  cmd(0x2C);        //VCOM Voltage
  IO.data(0x50);    
  cmd(0x03);        //Gate Driving voltage Control
  IO.data(0x15);    // 19V
  cmd(0x04);        //Source Driving voltage Control
  IO.data(0x41);    // VSH1 15V
  IO.data(0xA8);    // VSH2 5V
  IO.data(0x32);    // VSL -15V

  cmd(0x3A); //Dummy Line
  IO.data(0x2C);
  cmd(0x3B); //Gate time
  IO.data(0x0B);
  cmd(0x4E); // set RAM x address count to 0;
  IO.data(0x00);
  cmd(0x4F); // set RAM y address count to 0X127;
  IO.data(0xF9);
  IO.data(0x00);

  _setRamDataEntryMode(0x03);
}


void Gdeh0213b73::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  if (debug_enabled) printf("_SetRamArea(xS:%d,xE:%d,Ys:%d,Y1s:%d,Ye:%d,Ye1:%d)\n",Xstart,Xend,Ystart,Ystart1,Yend,Yend1);
  cmd(0x44);
  IO.data(Xstart);
  IO.data(Xend);
  cmd(0x45);
  IO.data(Ystart);
  IO.data(Ystart1);
  IO.data(Yend);
  IO.data(Yend1);
}

void Gdeh0213b73::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  if (debug_enabled)  printf("_SetRamPointer(addrX:%d,addrY:%d,addrY1:%d)\n",addrX,addrY,addrY1);
  cmd(0x4e);
  IO.data(addrX);
  cmd(0x4f);
  IO.data(addrY);
  IO.data(addrY1);
}

//We use only 0x03
//ram_entry_mode = 0x03; // y-increment, x-increment : normal mode
//ram_entry_mode = 0x00; // y-decrement, x-decrement
//ram_entry_mode = 0x01; // y-decrement, x-increment
//ram_entry_mode = 0x02; // y-increment, x-decrement
void Gdeh0213b73::_setRamDataEntryMode(uint8_t em)
{
  const uint16_t xPixelsPar = GDEH0213B73_X_PIXELS - 1;
  const uint16_t yPixelsPar = GDEH0213B73_Y_PIXELS - 1;
  em = gx_uint16_min(em, 0x03);
  cmd(0x11);
  IO.data(em);
  switch (em)
  {
    case 0x00: // x decrease, y decrease
      _SetRamArea(xPixelsPar / 8, 0x00, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x01: // x increase, y decrease : as in demo code
      _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
      _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
      break;
    case 0x02: // x decrease, y increase
      _SetRamArea(xPixelsPar / 8, 0x00, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(xPixelsPar / 8, 0x00, 0x00); // set ram
      break;
    case 0x03: // x increase, y increase : normal mode
      _SetRamArea(0x00, xPixelsPar / 8, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
      _SetRamPointer(0x00, 0x00, 0x00); // set ram
      break;
  }
}

void Gdeh0213b73::_powerOn()
{
  cmd(0x22);
  IO.data(0xc0);
  cmd(0x20);

  _waitBusy("_powerOn");
}
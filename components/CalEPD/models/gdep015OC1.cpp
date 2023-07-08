#include "gdep015OC1.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//full screen update LUT
const epd_init_30 Gdep015OC1::LUTDefault_full={
0x32, {
  0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},30};

const epd_init_30 Gdep015OC1::LUTDefault_part={
0x32, {
  0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},30};

DRAM_ATTR const epd_init_3 Gdep015OC1::GDOControl={
0x01,{(GDEP015OC1_HEIGHT - 1) % 256, (GDEP015OC1_HEIGHT - 1) / 256, 0x00},3
};

DRAM_ATTR const epd_init_3 Gdep015OC1::epd_soft_start={
0x0c,{0xd7, 0xd6, 0x9d},3
};

DRAM_ATTR const epd_init_1 Gdep015OC1::VCOMVol={
0x2c,{0x9b},1
}; // VCOM 7c

DRAM_ATTR const epd_init_1 Gdep015OC1::DummyLine={
0x3a,{0x1a},1
};

DRAM_ATTR const epd_init_1 Gdep015OC1::Gatetime={
0x3b,{0x08},1
};

// Partial Update Delay
#define GDEP015OC1_PU_DELAY 100


// Constructor
Gdep015OC1::Gdep015OC1(EpdSpi& dio): 
  Adafruit_GFX(GDEP015OC1_WIDTH, GDEP015OC1_HEIGHT),
  Epd(GDEP015OC1_WIDTH, GDEP015OC1_HEIGHT), IO(dio)
{
  printf("Gdep015OC1() %d*%d\n",
  GDEP015OC1_WIDTH, GDEP015OC1_HEIGHT);  
}

void Gdep015OC1::initFullUpdate(){
    _wakeUp(0x03);
    
    IO.cmd(LUTDefault_full.cmd);     // boost
    for (int i=0;i<LUTDefault_full.databytes;++i) {
        IO.data(LUTDefault_full.data[i]);
    } 
    _PowerOn();
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

void Gdep015OC1::initPartialUpdate(){
    _wakeUp(0x03);

    IO.cmd(LUTDefault_part.cmd);     // boost
    for (int i=0;i<LUTDefault_part.databytes;++i) {
        IO.data(LUTDefault_part.data[i]);
    }
    _PowerOn();

    if (debug_enabled) printf("initPartialUpdate() LUT\n");
}

//Initialize the display
void Gdep015OC1::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdep015OC1::init(%d)\n", debug);
    IO.init(4, debug); // 4MHz frequency

    printf("Free heap:%d\n", (int)xPortGetFreeHeapSize());
    fillScreen(EPD_WHITE);
}

void Gdep015OC1::fillScreen(uint16_t color)
{
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  uint8_t data = (color == EPD_BLACK) ? GDEP015OC1_8PIX_BLACK : GDEP015OC1_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

uint16_t Gdep015OC1::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  printf("_setPartialRamArea not used in Gdep015OC1");
  return 0;
}
void Gdep015OC1::_wakeUp(){
  printf("_wakeUp not used in Gdep015OC1");
}

/**
 * @deprecated It seems there is no need to do this for now
 */
void Gdep015OC1::_writeCommandData(const uint8_t cmd, const uint8_t* pCommandData, uint8_t datalen) {
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY)){
    _waitBusy("_waitBusy",100);
  }
  IO.cmd(cmd);
  for (int i=0;i<datalen;++i) {
      IO.data(*pCommandData++);
  }
}

void Gdep015OC1::_wakeUp(uint8_t em){
  printf("wakeup() start commands\n");

  IO.cmd(GDOControl.cmd);
  for (int i=0;i<GDOControl.databytes;++i) {
      IO.data(GDOControl.data[i]);
  }

  IO.cmd(epd_soft_start.cmd);     // boost
  for (int i=0;i<epd_soft_start.databytes;++i) {
      IO.data(epd_soft_start.data[i]);
  }

  IO.cmd(VCOMVol.cmd);
  IO.data(VCOMVol.data[0]);

  _waitBusy("epd_wakeup_power:ON"); // Needed?

  IO.cmd(DummyLine.cmd);
  IO.data(DummyLine.data[0]);
  
  IO.cmd(Gatetime.cmd);            // CMD: 0x30 DATA: 0xbf
  IO.data(Gatetime.data[0]);

  _setRamDataEntryMode(em);
}

void Gdep015OC1::update()
{
  initFullUpdate();
  printf("BUFF Size:%d\n",sizeof(_buffer));

  IO.cmd(0x24);        // update current data
  for (uint16_t y = 0; y < GDEP015OC1_HEIGHT; y++)
  {
    for (uint16_t x = 0; x < GDEP015OC1_WIDTH / 8; x++)
    {
      uint16_t idx = y * (GDEP015OC1_WIDTH / 8) + x;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  IO.cmd(0x22);
  IO.data(0xc4);
  IO.cmd(0x20);
  _waitBusy("_Update_Full", 1200);
  IO.cmd(0xff);

  _sleep();
}

void Gdep015OC1::_setRamDataEntryMode(uint8_t em)
{
  const uint16_t xPixelsPar = GDEP015OC1_WIDTH - 1;
  const uint16_t yPixelsPar = GDEP015OC1_HEIGHT - 1;
  em = gx_uint16_min(em, 0x03);
  IO.cmd(0x11);
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

void Gdep015OC1::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  IO.cmd(0x44);
  IO.data(Xstart);
  IO.data(Xend);
  IO.cmd(0x45);
  IO.data(Ystart);
  IO.data(Ystart1);
  IO.data(Yend);
  IO.data(Yend1);
}

void Gdep015OC1::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  IO.cmd(0x4e);
  IO.data(addrX);
  IO.cmd(0x4f);
  IO.data(addrY);
  IO.data(addrY1);
}

void Gdep015OC1::_PowerOn(void)
{
  IO.cmd(0x22);
  IO.data(0xc0);
  IO.cmd(0x20);
  _waitBusy("_PowerOn");
}

void Gdep015OC1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEP015OC1_WIDTH) {
    printf("x:%d exceeded boundary %d\n",x,WIDTH);
    return;
  }
  if (y >= GDEP015OC1_HEIGHT) {
    printf("y:%d exceeded boundary %d\n",y,HEIGHT);
    return;
  }
  uint16_t xe = gx_uint16_min(GDEP015OC1_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEP015OC1_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8;
  initPartialUpdate();
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  _waitBusy("partialUpdate1", 100); // needed ?
  IO.cmd(0x24);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEP015OC1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  
  IO.cmd(0x22);
  IO.data(0x04);
  IO.cmd(0x20);
  _waitBusy("partialUpdate2", 300);
  IO.cmd(0xff);

  vTaskDelay(GDEP015OC1_PU_DELAY/portTICK_PERIOD_MS);

  // update erase buffer
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  _waitBusy("partialUpdate3", 100); // needed ?
  IO.cmd(0x24);

  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEP015OC1_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  vTaskDelay(GDEP015OC1_PU_DELAY/portTICK_PERIOD_MS);
}

void Gdep015OC1::_waitBusy(const char* message, uint16_t busy_time){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // On high is busy
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  } else {
    vTaskDelay(busy_time/portTICK_PERIOD_MS);
  }
}

void Gdep015OC1::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    // On low is not busy anymore
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdep015OC1::_sleep(){
  IO.cmd(0x22); // power off display
  IO.data(0xc3);
  IO.cmd(0x20);
  _waitBusy("power_off");
}

void Gdep015OC1::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEP015OC1_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEP015OC1_WIDTH - x - w - 1;
      y = GDEP015OC1_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEP015OC1_HEIGHT - y - h - 1;
      break;
  }
}


void Gdep015OC1::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEP015OC1_WIDTH - x - 1;
      break;
    case 2:
      x = GDEP015OC1_WIDTH - x - 1;
      y = GDEP015OC1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEP015OC1_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEP015OC1_WIDTH / 8;

  // This is the trick to draw colors right. Genious Jean-Marc
  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
}

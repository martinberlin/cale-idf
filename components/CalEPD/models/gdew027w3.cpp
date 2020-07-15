#include "gdew027w3.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

/*
History & fail record: https://twitter.com/martinfasani/status/1265762052880175107
*/

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//full screen update LUT
const epd_init_44 Gdew027w3::lut_20_vcomDC={
0x20, {
  0x00, 0x00,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},44};

const epd_init_42 Gdew027w3::lut_21_ww={
0x21, {
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

const epd_init_42 Gdew027w3::lut_22_bw={
0x22,{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

const epd_init_42 Gdew027w3::lut_23_wb={
0x23,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

const epd_init_42 Gdew027w3::lut_24_bb={
0x24,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_power_4 Gdew027w3::epd_wakeup_power={
0x01,{0x03,0x00,0x2b,0x2b},4
};

DRAM_ATTR const epd_init_3 Gdew027w3::epd_soft_start={
0x06,{0x07,0x07,0x17},3
};

DRAM_ATTR const epd_init_1 Gdew027w3::epd_extra_setting={
0x16,{0x00},1
};

DRAM_ATTR const epd_init_1 Gdew027w3::epd_panel_setting={
0x00,{0xbf},1
};

DRAM_ATTR const epd_init_1 Gdew027w3::epd_pll={
0x30,{0x3a},1
};

DRAM_ATTR const epd_init_4 Gdew027w3::epd_resolution={
0x61,{0x00, 0xb0, /*176*/ 0x01, 0x08 // 264
},4};

DRAM_ATTR const epd_init_1 Gdew027w3::epd_vcom1={
0x82,{0x08},1
};

DRAM_ATTR const epd_init_1 Gdew027w3::epd_vcom2={
0x50,{0x97},1
};

//partial screen update LUT
DRAM_ATTR const epd_init_44 Gdew027w3::lut_20_vcomDC_partial={
0x20, {
  0x00, 0x00,
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},44};

DRAM_ATTR const epd_init_42 Gdew027w3::lut_21_ww_partial={
0x21, {
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 Gdew027w3::lut_22_bw_partial={
0x22, {
  0x80, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 Gdew027w3::lut_23_wb_partial={
0x23, {
  0x40, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 Gdew027w3::lut_24_bb_partial={
0x24, {
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

// Partial Update Delay, may have an influence on degradation
#define GDEW027W3_PU_DELAY 100


// Constructor
Gdew027w3::Gdew027w3(EpdSpi& dio): 
  Adafruit_GFX(GDEW027W3_WIDTH, GDEW027W3_HEIGHT),
  Epd(GDEW027W3_WIDTH, GDEW027W3_HEIGHT), IO(dio)
{
  printf("Gdew027w3() %d*%d\n",
  GDEW027W3_WIDTH, GDEW027W3_HEIGHT);  

  //printf("\n\n\n____________DO NOT USE: https://twitter.com/martinfasani/status/1265762052880175107");
}

void Gdew027w3::initFullUpdate(){
    IO.cmd(0x82);  //vcom_DC setting
    IO.data(0x08);

    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(0x97); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57

    IO.cmd(lut_20_vcomDC.cmd);
    // Sending LUTs like this for this controller was making all arrive with 0's
    //IO.data(lut_20_vcomDC.data,lut_20_vcomDC.databytes);
    for (int i=0;i<lut_20_vcomDC.databytes;++i) {
      IO.data(lut_20_vcomDC.data[i]);
     }
   
    IO.cmd(lut_21_ww.cmd);
    for (int i=0;i<lut_21_ww.databytes;++i) {
      IO.data(lut_21_ww.data[i]);
     }

    IO.cmd(lut_22_bw.cmd);
    for (int i=0;i<lut_22_bw.databytes;++i) {
      IO.data(lut_22_bw.data[i]);
     }
    IO.cmd(lut_23_wb.cmd);
    for (int i=0;i<lut_23_wb.databytes;++i) {
      IO.data(lut_23_wb.data[i]);
     }
     

    IO.cmd(lut_24_bb.cmd);
    for (int i=0;i<lut_24_bb.databytes;++i) {
      IO.data(lut_24_bb.data[i]);
     }
     
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

void Gdew027w3::initPartialUpdate(){
    IO.cmd(0x82);  //vcom_DC setting
    IO.data(0x08);

    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(0x17); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

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
void Gdew027w3::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew027w3::init(%d)\n", debug);
    IO.init(4, debug); // 4MHz frequency

    printf("Free heap:%d\n",xPortGetFreeHeapSize());
    fillScreen(EPD_WHITE);
}

void Gdew027w3::fillScreen(uint16_t color)
{
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  uint8_t data = (color == EPD_BLACK) ? GDEW027W3_8PIX_BLACK : GDEW027W3_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

void Gdew027w3::_wakeUp(){
  printf("wakeup() start commands\n");
  IO.reset(10);

  IO.cmd(epd_wakeup_power.cmd);   // power setting
  for (int i=0;i<epd_wakeup_power.databytes;++i) {
      IO.data(epd_wakeup_power.data[i]);
  }

  IO.cmd(epd_soft_start.cmd);     // boost
  for (int i=0;i<epd_soft_start.databytes;++i) {
      IO.data(epd_soft_start.data[i]);
  }

  IO.cmd(epd_extra_setting.cmd);  // CMD: 0x16 DATA: 0x00
  IO.data(epd_extra_setting.data[0]);
  
  IO.cmd(0x04);
  _waitBusy("epd_wakeup_power:ON");

  IO.cmd(epd_panel_setting.cmd);  // CMD: 0x00 DATA: 0xbf
  IO.data(epd_panel_setting.data[0]);
  
                                  // >3a 100HZ   29 150Hz 39 200HZ 31 171HZ EPD RE
  IO.cmd(epd_pll.cmd);            // CMD: 0x30 DATA: 0xbf
  IO.data(epd_pll.data[0]);
   
  //resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<epd_resolution.databytes;++i) {
      IO.data(epd_resolution.data[i]);
  }
  
  IO.cmd(epd_vcom1.cmd);          // vcom_DC 0x28:-2.0V,0x12:-0.9V
  IO.data(epd_vcom1.data[0]);     // CMD: 0x82 DATA: 0x08 

  vTaskDelay(2/portTICK_RATE_MS); // delay(2)

  //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
  IO.cmd(epd_vcom2.cmd);          // vcom and data interval
  IO.data(epd_vcom2.data[0]);     // CMD: 0x50 DATA: 0x97 

  initFullUpdate();
}

void Gdew027w3::update()
{
  _wakeUp();
  printf("BUFF Size:%d\n",sizeof(_buffer));

  if (_initial) {       // init clean old data
    IO.cmd(0x10);
    for (uint16_t x = 0; x < GDEW027W3_BUFFER_SIZE; x++){
      IO.data(0xFF);
    }
    _initial=false;
  } 

  IO.cmd(0x13);        // update current data
  for (uint16_t x = 0; x < GDEW027W3_BUFFER_SIZE; x++){
    if (debug_enabled){
      if (x < 40) {
        printf("%x ",_buffer[x]);
      }
    }
    uint8_t pixel = sizeof(_buffer) ? ~_buffer[x] : 0xFF;
    IO.data(pixel);
  } 

  IO.cmd(0x12);        // display refresh
  _waitBusy("update"); 

  IO.cmd(0x10);       // update old data
  for (uint16_t x = 0; x < GDEW027W3_BUFFER_SIZE; x++){
    uint8_t pixel = sizeof(_buffer) ? ~_buffer[x] : 0xFF;
    IO.data(pixel);
  }

  _sleep();
}

uint16_t Gdew027w3::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  IO.data(x >> 8);
  IO.data(x & 0xf8);
  IO.data(y >> 8);
  IO.data(y & 0xff);
  IO.data(xe >> 8);
  IO.data(xe & 0xf8);
  IO.data(ye >> 8);
  IO.data(ye & 0xff);
 return 1;
}

void Gdew027w3::_partialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  IO.cmd(command);
  _setPartialRamArea(x,y,w,h);
}

void Gdew027w3::_refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  w += (x % 8) + 7;
  h = gx_uint16_min(h, 256); // strange controller error
  IO.cmd(0x16);
  IO.data(x >> 8);
  IO.data(x & 0xf8);
  IO.data(y >> 8);
  IO.data(y & 0xff);
  IO.data(w >> 8);
  IO.data(w & 0xf8);
  IO.data(h >> 8);
  IO.data(h & 0xff);
}

void Gdew027w3::_writeToWindow(uint8_t command, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= GDEW027W3_WIDTH) return;
  if (ys >= GDEW027W3_HEIGHT) return;
  if (xd >= GDEW027W3_WIDTH) return;
  if (yd >= GDEW027W3_HEIGHT) return;
  w = gx_uint16_min(w + 7, GDEW027W3_WIDTH - xd) + (xd % 8);
  h = gx_uint16_min(h, GDEW027W3_HEIGHT - yd);
  uint16_t xe = (xs / 8) + (w / 8);
  IO.cmd(0x91); // partial in
  _partialRamArea(command, xd, yd, w, h);
  for (uint16_t y1 = ys; y1 < ys + h; y1++)
  {
    for (uint16_t x1 = xs / 8; x1 < xe; x1++)
    {
      uint16_t idx = y1 * (GDEW027W3_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  //delay(2);
}

void Gdew027w3::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;

  //TODO: Test this in T5S
  initPartialUpdate();
  _writeToWindow(0x15, x, y, x, y, w, h);
  _refreshWindow(x, y, w, h); 
  
  _waitBusy("updateWindow");
  // leave both controller buffers equal
  _writeToWindow(0x14, x, y, x, y, w, h);
}

void Gdew027w3::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew027w3::_sleep(){
  IO.cmd(0x02); // power off display
  _waitBusy("power_off");
  IO.cmd(0x07); // deep sleep
  IO.data(0xa5);
}

void Gdew027w3::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW027W3_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW027W3_WIDTH - x - w - 1;
      y = GDEW027W3_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW027W3_HEIGHT - y - h - 1;
      break;
  }
}


void Gdew027w3::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW027W3_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW027W3_WIDTH - x - 1;
      y = GDEW027W3_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW027W3_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW027W3_WIDTH / 8;

  // This is the trick to draw colors right. Genious Jean-Marc
  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
}

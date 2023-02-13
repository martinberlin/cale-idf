#include "gdem029E97.h"
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
#define GDEM029E97_PU_DELAY 300

const unsigned char LUT_DATA[]= {
0x80,0x60,0x40,0x00,0x00,0x00,0x00,             //LUT0: BB:     VS 0 ~7
0x10,0x60,0x20,0x00,0x00,0x00,0x00,             //LUT1: BW:     VS 0 ~7
0x80,0x60,0x40,0x00,0x00,0x00,0x00,             //LUT2: WB:     VS 0 ~7
0x10,0x60,0x20,0x00,0x00,0x00,0x00,             //LUT3: WW:     VS 0 ~7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT4: VCOM:   VS 0 ~7

0x03,0x03,0x00,0x00,0x02,                       // TP0 A~D RP0
0x09,0x09,0x00,0x00,0x02,                       // TP1 A~D RP1
0x03,0x03,0x00,0x00,0x02,                       // TP2 A~D RP2
0x00,0x00,0x00,0x00,0x00,                       // TP3 A~D RP3
0x00,0x00,0x00,0x00,0x00,                       // TP4 A~D RP4
0x00,0x00,0x00,0x00,0x00,                       // TP5 A~D RP5
0x00,0x00,0x00,0x00,0x00,                       // TP6 A~D RP6

0x15,0x41,0xA8,0x32,0x30,0x0A,
};

const unsigned char LUT_DATA_part[]={  //20 bytes
0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT0: BB:     VS 0 ~7
0x80,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT1: BW:     VS 0 ~7
0x40,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT2: WB:     VS 0 ~7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT3: WW:     VS 0 ~7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,             //LUT4: VCOM:   VS 0 ~7

0x0A,0x00,0x00,0x00,0x00,                       // TP0 A~D RP0
0x00,0x00,0x00,0x00,0x00,                       // TP1 A~D RP1
0x00,0x00,0x00,0x00,0x00,                       // TP2 A~D RP2
0x00,0x00,0x00,0x00,0x00,                       // TP3 A~D RP3
0x00,0x00,0x00,0x00,0x00,                       // TP4 A~D RP4
0x00,0x00,0x00,0x00,0x00,                       // TP5 A~D RP5
0x00,0x00,0x00,0x00,0x00,                       // TP6 A~D RP6

0x15,0x41,0xA8,0x32,0x30,0x0A,
};				


// Constructor GDEY0213B74
Gdem029E97::Gdem029E97(EpdSpi& dio): 
  Adafruit_GFX(GDEM029E97_WIDTH, GDEM029E97_HEIGHT),
  Epd(GDEM029E97_WIDTH, GDEM029E97_HEIGHT), IO(dio)
{
  printf("Gdem029E97() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEM029E97_WIDTH, GDEM029E97_HEIGHT);  
}

//Initialize the display
void Gdem029E97::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdem029E97::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdem029E97::fillScreen(uint16_t color)
{
    uint8_t data = (color == EPD_WHITE) ?  0xFF : 0x00;
    for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
    {
      _mono_buffer[x] = data;
    }
    if (debug_enabled) printf("fillScreen(%d) _mono_buffer len:%d\n",data,sizeof(_mono_buffer));
}


void Gdem029E97::update()
{
  _using_partial_mode = false;
  uint64_t startTime = esp_timer_get_time();
  
  // For v1.0 only monochrome supported
  uint8_t xLineBytes = GDEM029E97_WIDTH/8;
  uint8_t x1buf[xLineBytes];
  uint32_t i = 0;

  _wakeUp();

  IO.cmd(0x24); // write RAM1 for black(0)/white (1)
  for (int y = GDEM029E97_HEIGHT-1; y >= 0; y--) {
    for (uint16_t x = 0; x < xLineBytes; x++)
    {
      uint16_t idx = y * xLineBytes + x;
      // No need to safeward this if GDEM029E97_HEIGHT & xLineBytes are well calculated
      //uint8_t data = i < sizeof(_mono_buffer) ? _mono_buffer[idx] : 0x00;
      x1buf[x] = _mono_buffer[idx];

      if (x==xLineBytes-1) { // Flush the X line buffer to SPI
            IO.data(x1buf,sizeof(x1buf));
          }
      ++i;
    }
  }  
  // This 2 should match or we are doing something wrong
  printf("Buffer size: %d sent: %d\n ", (int)GDEM029E97_BUFFER_SIZE, (int)i );
  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x20);        // Update sequence

  _waitBusy("update full");
  uint64_t powerOnTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

void Gdem029E97::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (!_using_partial_mode) {
    _using_partial_mode = true;
    _wakeUpPartial();
    IO.cmd(0x26); // RAM2 clean it up at the start
    for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
    {
      IO.data(0xFF);
    }
  }
  
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEM029E97_WIDTH) return;
  if (y >= GDEM029E97_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEM029E97_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEM029E97_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8;

  partials++;
  ESP_LOGI("PARTIAL", "partial %d x:%d y:%d\n", partials, (int)x, (int)y);

  _setRamDataEntryMode(0x03);
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  
  IO.cmd(0x24); // BW RAM
  //printf("Loop from ys:%d to ye:%d\n", y, ye);
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEM029E97_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_mono_buffer)) ? _mono_buffer[idx] : 0x00;
      IO.data(data);
    }
  }
  IO.cmd(0x22);
  IO.data(0x0C); // 0xFC in GxEPD class
  IO.cmd(0x20);
  //vTaskDelay(pdMS_TO_TICKS(GDEM029E97_PU_DELAY));
  _waitBusy("partial");
}

void Gdem029E97::_waitBusy(const char* message){
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

void Gdem029E97::_sleep(){
  IO.cmd(0x22); //POWER OFF
	IO.data(0xC3);   
	IO.cmd(0x20);  
	
  IO.cmd(0x10); //enter deep sleep
  IO.data(0x01); 
}

void Gdem029E97::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
   switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEM029E97_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEM029E97_WIDTH - x - w - 1;
      y = GDEM029E97_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEM029E97_HEIGHT - y - h - 1;
      break;
  }
}

void Gdem029E97::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEM029E97_WIDTH - x -1;
      break;
    case 2:
      x = GDEM029E97_WIDTH - x -1;
      y = GDEM029E97_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEM029E97_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEM029E97_WIDTH / 8;
  uint8_t mask = 1 << (7 - x % 8);
  if (color) {
      _mono_buffer[i] = _mono_buffer[i] | mask;
    } else {
      _mono_buffer[i] = _mono_buffer[i] & (0xFF ^ mask);
    }
}

void Gdem029E97::_wakeUp(){
  IO.reset(10);
  _waitBusy("RST reset");
  IO.cmd(0x12); //SWRESET
  _waitBusy("SWRESET");

  //IO.cmd(0x01); //Driver output control      
  //IO.data(0x27);
  IO.cmd(0x74); //set analog block control       
	IO.data(0x54);
	IO.cmd(0x7E); //set digital block control          
	IO.data(0x3B);

	IO.cmd(0x01); //Driver output control      
	IO.data(0x27);
	IO.data(0x01);
	IO.data(0x00);

	IO.cmd(0x11); //data entry mode       
	IO.data(0x01);

	IO.cmd(0x44); //set Ram-X address start/end position   
	IO.data(0x00);
	IO.data(0x0F);    //0x0C-->(15+1)*8=128

	IO.cmd(0x45); //set Ram-Y address start/end position          
	IO.data(0x27);   //0x0127-->(295+1)=296
	IO.data(0x01);
	IO.data(0x00);
	IO.data(0x00); 

	IO.cmd(0x3C); //BorderWavefrom
	IO.data(0x03);	

	IO.cmd(0x2C);     //VCOM Voltage
	IO.data(vcom);    //

	IO.cmd(0x03);
	IO.data(LUT_DATA[70]);

	IO.cmd(0x04);     
	IO.data(LUT_DATA[71]);    
	IO.data(LUT_DATA[72]);    
	IO.data(LUT_DATA[73]);    

  IO.cmd(0x32); // Send LUTs
  IO.data(LUT_DATA, 70);

	//IO.cmd(0x3A);     //Dummy Line 	 
	//IO.data(LUT_DATA[74]);  
	IO.cmd(0x3B);     //Gate time 
	IO.data(LUT_DATA[75]);
  _waitBusy("wakeup CMDs");
}

void Gdem029E97::_wakeUpPartial(){
  IO.cmd(0x12);  //SWRESET
  _waitBusy("SWRESET");

  IO.cmd(0x32); // Send LUTs
  IO.data(LUT_DATA_part, 70);

  IO.cmd(0x37); 
  IO.data(0x00);  
  IO.data(0x00);  
  IO.data(0x00);  
  IO.data(0x00);  
  IO.data(0x40);  
  IO.data(0x00);  
  IO.data(0x00);   
	
  IO.cmd(0x3C); //BorderWavefrom
	IO.data(0x01);
IO.cmd(0x22); 
  IO.data(0xC0); 
  IO.cmd(0x20); 
   _waitBusy("Power on");
}

void Gdem029E97::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
  if (debug_enabled) {
    printf("_SetRamArea(xS:%d,xE:%d,Ys:%d,Y1s:%d,Ye:%d,Ye1:%d)\n",Xstart,Xend,Ystart,Ystart1,Yend,Yend1);
  }
  IO.cmd(0x44);
  IO.data(Xstart);
  IO.data(Xend);
  IO.cmd(0x45);
  IO.data(Ystart);
  IO.data(Ystart1);
  IO.data(Yend);
  IO.data(Yend1);
}

void Gdem029E97::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
  if (debug_enabled) {
   printf("_SetRamPointer(addrX:%d,addrY:%d,addrY1:%d)\n",addrX,addrY,addrY1);
  }
  IO.cmd(0x4e);
  IO.data(addrX);
  IO.cmd(0x4f);
  IO.data(addrY);
  IO.data(addrY1);
}

//We use only 0x03: At the moment this method is not used
//ram_entry_mode = 0x03; // y-increment, x-increment : normal mode
//ram_entry_mode = 0x00; // y-decrement, x-decrement
//ram_entry_mode = 0x01; // y-decrement, x-increment
//ram_entry_mode = 0x02; // y-increment, x-decrement
void Gdem029E97::_setRamDataEntryMode(uint8_t em)
{
  const uint16_t xPixelsPar = GDEM029E97_X_PIXELS - 1;
  const uint16_t yPixelsPar = GDEM029E97_Y_PIXELS - 1;
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

/**
 * @brief Sets private _mode. When true is monochrome mode
 */
void Gdem029E97::setMonoMode(bool mode) {
  ESP_LOGE("Gdem029E97", "This epaper has no 4 gray mode");
}

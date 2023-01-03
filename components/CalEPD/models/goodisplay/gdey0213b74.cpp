#include "goodisplay/gdey0213b74.h"
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

// Grays Waveform
const epd_lut_159 Gdey0213b74::lut_4_grays={
0x32, {
  0x40,	0x48,	0x80,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x8,	0x48,	0x10,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x2,	0x48,	0x4,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x20,	0x48,	0x1,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0xA,	0x19,	0x0,	0x3,	0x8,	0x0,	0x0,					
0x14,	0x1,	0x0,	0x14,	0x1,	0x0,	0x3,					
0xA,	0x3,	0x0,	0x8,	0x19,	0x0,	0x0,					
0x1,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	0x0,	0x0,	0x0,			
0x22,	0x17,	0x41,	0x0,	0x32,	0x1C
},159};

// Constructor GDEY0213B74
Gdey0213b74::Gdey0213b74(EpdSpi& dio): 
  Adafruit_GFX(GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT),
  Epd(GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT), IO(dio)
{
  printf("Gdey0213b74() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEH0213B73_WIDTH, GDEH0213B73_HEIGHT);  
}

//Initialize the display
void Gdey0213b74::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdey0213b74::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
    _mono_mode = 1;
    fillScreen(EPD_WHITE);
}

void Gdey0213b74::fillScreen(uint16_t color)
{
  if (_mono_mode) {
    uint8_t data = (color == EPD_WHITE) ?  0xFF : 0x00;
    for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
    {
      _mono_buffer[x] = data;
    }
    if (debug_enabled) printf("fillScreen(%d) _mono_buffer len:%d\n",data,sizeof(_mono_buffer));
  } else {

    // This is to make faster black & white
    if (color == 255 || color == 0) {
      for(uint32_t i=0;i<GDEH0213B73_BUFFER_SIZE;i++)
      {
        _buffer1[i] = (color == 0xFF) ? 0x00 : 0xFF;
        _buffer2[i] = (color == 0xFF) ? 0x00 : 0xFF;
      }
    return;
     }
   
    for (uint32_t y = 0; y < GDEH0213B73_HEIGHT; y++)
    {
      for (uint32_t x = 0; x < GDEH0213B73_WIDTH; x++)
      {
        drawPixel(x, y, color);
        if (x % 8 == 0)
          {
            vTaskDelay(pdMS_TO_TICKS(2));
          }
      }
    }
  }

}


void Gdey0213b74::update()
{
  _using_partial_mode = false;
  uint64_t startTime = esp_timer_get_time();
  
  // For v1.0 only monochrome supported
  uint8_t xLineBytes = GDEH0213B73_WIDTH/8;
  uint8_t x1buf[xLineBytes];
  uint32_t i = 0;
 
  if (_mono_mode) {
    _wakeUp();

    IO.cmd(0x24); // write RAM1 for black(0)/white (1)
    for (uint16_t y = GDEH0213B73_HEIGHT; y >0; y--) {
      for (uint16_t x = 0; x < xLineBytes; x++)
      {
        uint16_t idx = y * xLineBytes + x;
        uint8_t data = i < sizeof(_mono_buffer) ? _mono_buffer[idx] : 0x00;
        x1buf[x] = data; // ~ is invert

        if (x==xLineBytes-1) { // Flush the X line buffer to SPI
              IO.data(x1buf,sizeof(x1buf));
            }
        ++i;
      }
    }

  } else {
    _wakeUpGrayMode();
    
    // 4 grays mode GDEH0213B73
    printf("\n4 gray MODE. sends LUT 159 bytes\n");
    IO.cmd(0x24); // write RAM1 for black(0)/white (1)
    for (uint16_t y = GDEH0213B73_HEIGHT; y > 0; y--) {
      for (uint16_t x = 0; x < xLineBytes; x++)
      {
        uint16_t idx = y * xLineBytes + x;
        uint8_t data = i < sizeof(_buffer1) ? _buffer1[idx] : 0x00;
        x1buf[x] = data; // ~ is invert

        if (x==xLineBytes-1) { // Flush the X line buffer to SPI
              IO.data(x1buf,sizeof(x1buf));
            }
        ++i;
      }
    }
    
    i = 0;
    IO.cmd(0x26); //RAM2 buffer: SPI2
    for (uint16_t y = GDEH0213B73_HEIGHT; y > 0; y--) {
      for (uint16_t x = 0; x < xLineBytes; x++)
      {
        uint16_t idx = y * xLineBytes + x;
        uint8_t data = i < sizeof(_buffer2) ? _buffer2[idx] : 0x00;
        x1buf[x] = data; // ~ is invert

        if (x==xLineBytes-1) { // Flush the X line buffer to SPI
              IO.data(x1buf,sizeof(x1buf));
            }
        ++i;
      }
    }

  }
  uint64_t endTime = esp_timer_get_time();

  IO.cmd(0x22);        // Display Update Control
  uint8_t twenty_two = (_mono_mode) ? 0xF7 : 0xC4;
  IO.data(twenty_two); // When 4 gray 0xC7 : Same as gdeh042Z96
  IO.cmd(0x20);        // Update sequence

  _waitBusy("update full");
  uint64_t powerOnTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

void Gdey0213b74::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  //ESP_LOGE("PARTIAL", "update is not implemented x:%d y:%d\n", (int)x, (int)y);
  if (!_using_partial_mode) {
    _using_partial_mode = true;
    _wakeUp();
  }
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEH0213B73_WIDTH) return;
  if (y >= GDEH0213B73_HEIGHT) return;
  uint16_t xe = gx_uint16_min(GDEH0213B73_WIDTH, x + w) - 1;
  uint16_t ye = gx_uint16_min(GDEH0213B73_HEIGHT, y + h) - 1;
  uint16_t xs_d8 = x / 8;
  uint16_t xe_d8 = xe / 8;

  IO.cmd(0x12); //SWRESET
  _waitBusy("SWRESET");
  _setRamDataEntryMode(0x03);
  _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
  _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
  _waitBusy("updateWindow I");

  IO.cmd(0x22);
  IO.data(0xFF); 
  
  IO.cmd(0x24); // BW RAM
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEH0213B73_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_mono_buffer)) ? _mono_buffer[idx] : 0x00;
      IO.data(data);
    }
  }

  // If I don't do this then the 2nd partial comes out gray:
  IO.cmd(0x26); // RAM2
  for (int16_t y1 = y; y1 <= ye; y1++)
  {
    for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++)
    {
      uint16_t idx = y1 * (GDEH0213B73_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_mono_buffer)) ? _mono_buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  
  IO.cmd(0x20);
  _waitBusy("update partial");  
}

void Gdey0213b74::_waitBusy(const char* message){
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

void Gdey0213b74::_sleep(){
  IO.cmd(0x10); // deep sleep
  IO.data(0x01);
}

void Gdey0213b74::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
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

void Gdey0213b74::drawPixel(int16_t x, int16_t y, uint16_t color) {
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
  uint8_t mask = 1 << (7 - x % 8);

  if (_mono_mode) {
    if (color) {
        _mono_buffer[i] = _mono_buffer[i] | mask;
      } else {
        _mono_buffer[i] = _mono_buffer[i] & (0xFF ^ mask);
      }
  } else {
    // 4 gray mode
    mask = 0x80 >> (x & 7);
    color >>= 6; // Color is from 0 (black) to 255 (white)
    
    switch (color)
    {
      case 1:
        // Dark gray
        _buffer1[i] = _buffer1[i] & (0xFF ^ mask);
        _buffer2[i] = _buffer2[i] | mask;
        break;
      case 2:
        // Light gray
        _buffer1[i] = _buffer1[i] | mask;
        _buffer2[i] = _buffer2[i] & (0xFF ^ mask);
        break;
      case 3:
        // White
        _buffer1[i] = _buffer1[i] & (0xFF ^ mask);
        _buffer2[i] = _buffer2[i] & (0xFF ^ mask);
        break;
      default:
        // Black
        _buffer1[i] = _buffer1[i] | mask;
        _buffer2[i] = _buffer2[i] | mask;
        break;
      }
  }
}

// _InitDisplay generalizing names here
void Gdey0213b74::_wakeUp(){
  IO.reset(10);
  _waitBusy("RST reset");
  IO.cmd(0x12); //SWRESET
  _waitBusy("SWRESET");

  IO.cmd(0x01); //Driver output control      
  IO.data(0xF9);
  IO.data(0x00);
  IO.data(0x00);

  IO.cmd(0x11); //data entry mode       
  IO.data(0x01);

  IO.cmd(0x44); //set Ram-X address start/end position   
  IO.data(0x00);
  IO.data(0x0F);    //0x0F-->(15+1)*8=128

  IO.cmd(0x45); //set Ram-Y address start/end position          
  IO.data(0xF9);   //0xF9-->(249+1)=250
  IO.data(0x00);
  IO.data(0x00);
  IO.data(0x00);

  IO.cmd(0x3C); //BorderWavefrom
  IO.data(0x05);
  
  IO.cmd(0x21); //  Display update control
  IO.data(0x00);
  IO.data(0x80);
      
  IO.cmd(0x18); //Read built-in temperature sensor
  IO.data(0x80);

  IO.cmd(0x4E);   // set RAM x address count to 0;
  IO.data(0x00);
  IO.cmd(0x4F);   // set RAM y address count to 0X199;    
  IO.data(0xF9);
  IO.data(0x00);
  _waitBusy("wakeup CMDs");
}

void Gdey0213b74::_wakeUpGrayMode(){
  IO.reset(10);
  _waitBusy("RST reset");
  IO.cmd(0x12); //SWRESET
  _waitBusy("SWRESET");

  IO.cmd(0x74); //set analog block control       
	IO.data(0x54);
	IO.cmd(0x7E); //set digital block control          
	IO.data(0x3B);

	IO.cmd(0x01); //Driver output control      
	IO.data(0x07);
	IO.data(0x01);
	IO.data(0x00);

	IO.cmd(0x11); //data entry mode       
	IO.data(0x01);

  IO.cmd(0x44); //set Ram-X address start/end position   
  IO.data(0x00);
  IO.data(0x0F);    //0x0F-->(15+1)*8=128

  IO.cmd(0x45); //set Ram-Y address start/end position          
  IO.data(0xF9);   //0xF9-->(249+1)=250
  IO.data(0x00);
  IO.data(0x00);
  IO.data(0x00);

	IO.cmd(0x3C); //BorderWavefrom
	IO.data(0x00);	

	IO.cmd(0x2C);     //VCOM Voltage
	IO.data(lut_4_grays.data[158]);    //0x1C

	IO.cmd(0x3F); //EOPQ    
	IO.data(lut_4_grays.data[153]);
	
	IO.cmd(0x03); //VGH      
	IO.data(lut_4_grays.data[154]);

	IO.cmd(0x04); //      
	IO.data(lut_4_grays.data[155]); //VSH1   
	IO.data(lut_4_grays.data[156]); //VSH2   
	IO.data(lut_4_grays.data[157]); //VSL

  // LUT init table for 4 gray. Check if it's needed!
  IO.cmd(lut_4_grays.cmd);     // boost
  for (int i=0; i<lut_4_grays.databytes; ++i) {
      IO.data(lut_4_grays.data[i]);
  }
}

void Gdey0213b74::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
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

void Gdey0213b74::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
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
void Gdey0213b74::_setRamDataEntryMode(uint8_t em)
{
  const uint16_t xPixelsPar = GDEH0213B73_X_PIXELS - 1;
  const uint16_t yPixelsPar = GDEH0213B73_Y_PIXELS - 1;
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
void Gdey0213b74::setMonoMode(bool mode) {
  _mono_mode = mode;
}

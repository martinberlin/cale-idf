// Controller:        UC8253
// GDEQ037T31_416x240
#include "goodisplay/gdeq037T31.h"

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

//4Gray//////////////////////////////////////////
static const unsigned char GDEQ037T31_LUT[216]={							
0x01,	0x05,	0x20,	0x19,	0x0A,	0x01,	0x01,	
0x05,	0x0A,	0x01,	0x0A,	0x01,	0x01,	0x01,	
0x05,	0x09,	0x02,	0x03,	0x04,	0x01,	0x01,	
0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x05,	0x20,	0x19,	0x0A,	0x01,	0x01,	
0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,	
0x05,	0x49,	0x02,	0x83,	0x84,	0x01,	0x01,	
0x01,	0x84,	0x84,	0x82,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x05,	0x20,	0x99,	0x8A,	0x01,	0x01,	
0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,	
0x05,	0x49,	0x82,	0x03,	0x04,	0x01,	0x01,	
0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x85,	0x20,	0x99,	0x0A,	0x01,	0x01,	
0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,	
0x05,	0x49,	0x02,	0x83,	0x04,	0x01,	0x01,	
0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x85,	0xA0,	0x99,	0x0A,	0x01,	0x01,	
0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,	
0x05,	0x49,	0x02,	0x43,	0x04,	0x01,	0x01,	
0x01,	0x04,	0x04,	0x42,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,	
0x09,	0x10,	0x3F,	0x3F,	0x00,	0x0B,		
//Frame	VGH	VSH	VSL	VSHR	VCOM		
};

// Constructor
Gdeq037T31::Gdeq037T31(EpdSpi& dio): 
  Adafruit_GFX(GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT),
  Epd(GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT), IO(dio)
{
  printf("Gdeq037T31() %d*%d\n",
  GDEQ037T31_WIDTH, GDEQ037T31_HEIGHT);  
}

void Gdeq037T31::initFullUpdate(){
    _wakeUp();
    if (debug_enabled) printf("initFullUpdate() LUT\n");
}

//Initialize the display
void Gdeq037T31::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdeq037T31::init(%d)\n", debug);
    IO.init(4, debug); // 4MHz frequency

    printf("Free heap:%d\n", (int)xPortGetFreeHeapSize());
    fillScreen(EPD_WHITE);
    _mono_mode = 1;
    fillScreen(EPD_WHITE);
}

void Gdeq037T31::fillScreen(uint16_t color)
{
  if (_mono_mode) {
    // 0xFF = 8 pixels black, 0x00 = 8 pix. white
    uint8_t data = (color == EPD_BLACK) ? GDEQ037T31_8PIX_BLACK : GDEQ037T31_8PIX_WHITE;
    for (uint16_t x = 0; x < sizeof(_mono_buffer); x++)
    {
      _mono_buffer[x] = data;
      _buffer1[x] = data;
    }
  } else {
    uint8_t b1 = 0x00;
    uint8_t b2 = 0x00;
    switch (color)
    {
      case EPD_BLACK:
          b1 = 0xFF;
          b2 = 0xFF;
      break;
      case EPD_LIGHTGREY:
          b1 = 0xFF;
          b2 = 0x00;
      break;
      case EPD_DARKGREY:
          b1 = 0x00;
          b2 = 0xFF;
      break;
      case EPD_WHITE:
          b1 = 0x00;
          b2 = 0x00;
      break;
    }

      for(uint32_t i=0; i<GDEQ037T31_BUFFER_SIZE; i++)
      {
        _buffer1[i] = b1;
        _buffer2[i] = b2;
      }
    return;
     }

  if (debug_enabled) printf("fillScreen(%d) _mono_mode: %d len:%d\n",color,(int) _mono_mode,sizeof(_mono_buffer));
}

void Gdeq037T31::_wakeUp(){
  IO.reset(10);
  IO.cmd(0x04);
  _waitBusy("_PowerOn");

  if (_mono_mode) {
  IO.cmd(0x50);
  IO.data(0x97);
    // Marked in GOODISPLAY as EPD_init_Fast
    if (fast_mode) {
      IO.cmd(0xE0);
      IO.data(0x02);
      IO.cmd(0xE5);
      IO.data(0x5A);
    }
  } else {
    // 4 gray mode
    IO.cmd(0x00);//Panel Setting   
    IO.data(0xFF);//LUT FROM MCU
    IO.data(0x0D);

    IO.cmd(0x01);// power setting   
    IO.data(0x03);//Enable internal VSH\VSL\VGH\VGL	
    IO.data(GDEQ037T31_LUT[211]);//VGH=20V,VGL=-20V
    IO.data(GDEQ037T31_LUT[212]);//VSH=15V
    IO.data(GDEQ037T31_LUT[213]);//VSL=-15V
    IO.data(GDEQ037T31_LUT[214]);//VSHR

    IO.cmd(0x06);//booster soft start 
    IO.data(0xD7);//D7	
    IO.data(0xD7);//D7
    IO.data(0x27);//2F	

    IO.cmd(0x30);//PLL  -Frame rate 
    IO.data(GDEQ037T31_LUT[210]);//PLL
    
    IO.cmd(0x50); //CDI   
    IO.data(0x57);	

    IO.cmd(0x60); //TCON
    IO.data(0x22);//	

    IO.cmd(0x61); //Resolution	
    IO.data(0xF0);//HRES[7:3]-240
    IO.data(0x01);//VRES[15:8]	-320
    IO.data(0xA0);//VRES[7:0]
    
    IO.cmd(0x65); //Resolution	
    IO.data(0x00);//HRES[7:3]-240
    IO.data(0x00);//VRES[15:8]	-320
    IO.data(0x00);//VRES[7:0]	
    
    IO.cmd(0x82);//VCOM_DC   
    IO.data(GDEQ037T31_LUT[215]);//-2.0V	

    //Power Saving Register 
    IO.cmd(0xE3);        
    IO.data(0x88);//VCOM_W[3:0],SD_W[3:0]

    //LUT SETTING
    _writeFullLut();
  }
}

void Gdeq037T31::_writeFullLut() {
	unsigned int i;			
  /**
   * @brief This is horrible like this since it's raising CS pin for EVERY byte sent
   *        TODO: Refactor this in 4 different LUT constants and sent in a whole CS toggle
   */
	IO.cmd(0x20);// write VCOM register
	for(i=0;i<42;i++)
	{
			IO.data(GDEQ037T31_LUT[i]);
	}
	IO.cmd(0x21);// write LUTWW register
	for(i=42;i<84;i++)
	{
			IO.data(GDEQ037T31_LUT[i]);
	}
	IO.cmd(0x22);// write LUTR register
	for(i=84;i<126;i++)
	{
			IO.data(GDEQ037T31_LUT[i]);
	}
	IO.cmd(0x23);// write LUTW register
	for(i=126;i<168;i++)
	{
			IO.data(GDEQ037T31_LUT[i]);
	}
	IO.cmd(0x24);// write LUTB register
	for(i=168;i<210;i++)
	{	
			IO.data(GDEQ037T31_LUT[i]);	
	}
}

void Gdeq037T31::update()
{
  _partial_mode = false;
  uint64_t startTime = esp_timer_get_time();
  uint8_t xLineBytes = GDEQ037T31_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  _wakeUp();

  uint64_t endTime = esp_timer_get_time();
  if (_mono_mode) {
  if (total_updates) {
    // Old buffer update so the display can compare
    IO.cmd(0x10);
    for (uint16_t y = GDEQ037T31_HEIGHT; y > 0; y--)
      {
        for (uint16_t x = 0; x < xLineBytes; x++)
        {
          uint16_t idx = y * xLineBytes + x;  
          x1buf[x] = ~_buffer1[idx];
        }
        IO.data(x1buf, sizeof(x1buf));
      }
  }

  IO.cmd(0x13);
  for (uint16_t y = GDEQ037T31_HEIGHT; y > 0; y--)
    {
      for (uint16_t x = 0; x < xLineBytes; x++)
      {
        uint16_t idx = y * xLineBytes + x;  
        x1buf[x] = ~_mono_buffer[idx];
      }
      IO.data(x1buf, sizeof(x1buf));
    }
    total_updates++;
    memcpy(_buffer1, _mono_buffer, GDEQ037T31_BUFFER_SIZE);

    } else {
      // 4 Gray mode
      printf("4 Gray UPDATE\n\n");
      IO.cmd(0x13);
      for (int y = 0; y < GDEQ037T31_HEIGHT; y++)
        {
          for (int x = xLineBytes; x >= 0 ; x--)
          {
            uint16_t idx = y * xLineBytes + x;  
            x1buf[x] = ~_buffer1[idx];
          }
          IO.data(x1buf, sizeof(x1buf));
        }
      IO.cmd(0x10);
      for (int y = 0; y < GDEQ037T31_HEIGHT; y++)
        {
          for (int x = xLineBytes; x >= 0; x--)
          {
            uint16_t idx = y * xLineBytes + x;  
            x1buf[x] = ~_buffer2[idx];
          }
          IO.data(x1buf, sizeof(x1buf));
        }
    }

  IO.cmd(0x12);
  _waitBusy("_Update_Full");
  uint64_t powerOnTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);
  _sleep();
}

uint16_t Gdeq037T31::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
  x &= 0xFFF8;            // byte boundary
  xe = (xe - 1) | 0x0007; // byte boundary - 1
  IO.cmd(0x90);           // partial window
  IO.data(x / 256);
  IO.data(x % 256);        // x-start 
  IO.data(xe / 256);
  IO.data(xe % 256-1);     // x-end
  IO.data(y / 256);
  IO.data(y % 256);
  IO.data(ye / 256);
  IO.data(ye % 256-1);     // y-end
  
  return (7 + xe - x) / 8; // number of bytes to transfer per line
}

/**
 * @brief Partial update is NOT supported in 4 Grays mode
 * 
 */
void Gdeq037T31::initPartialUpdate(){
    printf("initPartialUpdate()\n");
    //_wakeUp(); // Does some partial refresh but completely wrong
    // https://github.com/GoodDisplay/E-paper-Display-Library-of-GoodDisplay/blob/main/Monochrome_E-paper-Display/3.7inch_GDEQ037T31_416x240/Display_EPD_W21.c#L76
    // Sending that referenced commmands
    IO.reset(10);
    IO.cmd(0x04);
    _waitBusy("_PowerOn");
    // Referenced from GD
    IO.cmd(0xE5);
    IO.data(0x6E);
 
    IO.cmd(0x50);
    IO.data(0xD7);
}

void Gdeq037T31::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  ESP_LOGI("Gdeq037T31", "Partial update is not yet implemented\n(We miss valid example, other UC's commands seem not to work here)");
  if (!_mono_mode) {
    ESP_LOGE("Gdeq037T31", "Partial update does not work in 4 gray mode");
    return;
  }
  if (! _partial_mode) { 
    initPartialUpdate();
    _partial_mode = true;
  }
  if (using_rotation) _rotate(x, y, w, h);
  if (x >= GDEQ037T31_WIDTH) {
    ESP_LOGE("updateWindow x", "Exceeds display width %d", GDEQ037T31_WIDTH);
    return;
  }
  if (y >= GDEQ037T31_HEIGHT) {
    ESP_LOGE("updateWindow y", "Exceeds display height %d", GDEQ037T31_HEIGHT);
    return;
  }
}

void Gdeq037T31::_waitBusy(const char* message){
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

void Gdeq037T31::_sleep(){
  IO.cmd(0X02); // power off display
  _waitBusy("power_off");
  IO.data(0X07);
  IO.cmd(0xA5);
}

void Gdeq037T31::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEQ037T31_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEQ037T31_WIDTH - x - w - 1;
      y = GDEQ037T31_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEQ037T31_HEIGHT - y - h - 1;
      break;
  }
}


void Gdeq037T31::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEQ037T31_WIDTH - x - 1;
      break;
    case 2:
      x = GDEQ037T31_WIDTH - x - 1;
      y = GDEQ037T31_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEQ037T31_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEQ037T31_WIDTH / 8;
  uint8_t mask = 1 << (7 - x % 8);
  
  if (_mono_mode) {

    if (color) {
      _mono_buffer[i] = (_mono_buffer[i] & (0xFF ^ mask));
      } else {
      _mono_buffer[i] = (_mono_buffer[i] | mask);
      }

  } else {
    // 4 gray mode
    //mask = 0x80 >> (x & 7);
    color >>= 6; // Color is from 0 (black) to 255 (white)
      
    switch (color)
      {
      case 1:
        // Dark gray: Correct
        _buffer1[i] = _buffer1[i] & (0xFF ^ mask);
        _buffer2[i] = _buffer2[i] | mask;
        break;
      case 2:
        // Light gray: Correct
        _buffer1[i] = _buffer1[i] | mask;
        _buffer2[i] = _buffer2[i] & (0xFF ^ mask);
        break;
      case 3:
        // Black
        _buffer1[i] = _buffer1[i] & (0xFF ^ mask);
        _buffer2[i] = _buffer2[i] & (0xFF ^ mask);
        break;
      default:
        // WHITE
        _buffer1[i] = _buffer1[i] | mask;
        _buffer2[i] = _buffer2[i] | mask;
        break;
      }
  }
}

void Gdeq037T31::setMonoMode(bool mode) {
  // Not sure if this is needed
  if (_mono_mode == true) {
    _mono_mode = false;
    fillScreen(EPD_WHITE); // Clean both buffers
    _mono_mode = true;
  }
  _mono_mode = mode;
}

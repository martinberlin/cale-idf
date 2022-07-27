#include "gdew042t2Grays.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

/*
 The EPD needs a bunch of command/data values to be initialized. They are send using the IO class
*/

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//C221 25C partial update waveform. Note: Partial does NOT work in 4 gray mode
DRAM_ATTR const epd_init_30 Gdew042t2Grays::lut_partial={
0x32, {
 0x10 ,0x18 ,0x18 ,0x08 ,0x18
,0x18 ,0x08 ,0x00 ,0x00 ,0x00
,0x00 ,0x00 ,0x00 ,0x00 ,0x00
,0x00 ,0x00 ,0x00 ,0x00 ,0x00
,0x13 ,0x14 ,0x44 ,0x12 ,0x00
,0x00 ,0x00 ,0x00 ,0x00 ,0x00
},30};

// Full screen update LUT 4 gray (Only full refresh)
DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_vcom11={
0x20, {
  0x00  ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
  0x60  ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
  0x00  ,0x14 ,0x00 ,0x00 ,0x00 ,0x01,
  0x00  ,0x13 ,0x0A ,0x01 ,0x00 ,0x01,
  0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
  0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
  0x00  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
},42};
// R21
DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_ww_full={
0x21, {
  0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
  0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
  0x10	,0x14	,0x0A	,0x00	,0x00	,0x01,
  0xA0	,0x13	,0x01	,0x00	,0x00	,0x01,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
},42};
// R22H  r
DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_bw_full={
0x22,{
  0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
  0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
  0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
  0x99	,0x0C	,0x01	,0x03	,0x04	,0x01,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
},42};
//R23H  w
DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_wb_full={
0x23,{
  0x40	,0x0A	,0x00	,0x00	,0x00	,0x01,
  0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
  0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
  0x99	,0x0B	,0x04	,0x04	,0x01	,0x01,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
},42};
//R24H  b
DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_bb_full={
0x24,{
  0x80	,0x0A	,0x00	,0x00	,0x00	,0x01,
  0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
  0x20	,0x14	,0x0A	,0x00	,0x00	,0x01,
  0x50	,0x13	,0x01	,0x00	,0x00	,0x01,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
  0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
},42};

// new waveform created by Jean-Marc Zingg for the actual panel
#define T1 25 // color change charge balance pre-phase
#define T2  1 // color change or sustain charge balance pre-phase
#define T3  2 // color change or sustain phase
#define T4 25 // color change phase

DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_21_ww_partial={
0x21,{
  0x18, T1, T2, T3, T4, 1, // 00 01 10 00
  0x00,  1,  0,  0,  0, 1, // gnd phase - 12 till here
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_22_bw_partial={
0x22,{ // 10 w
  0x5A, T1, T2, T3, T4, 1, // 01 01 10 10
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_23_wb_partial={
0x23,{
  0xA5, T1, T2, T3, T4, 1, // 10 10 01 01
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_init_42 Gdew042t2Grays::lut_24_bb_partial={
0x24,{
  0x24, T1, T2, T3, T4, 1, // 00 10 01 00
  0x00,  1,  0,  0,  0, 1, // gnd phase
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},42};

DRAM_ATTR const epd_power_4 Gdew042t2Grays::epd_wakeup_power={
0x01,{0x03,0x00,0x2b,0x2b},4
};

DRAM_ATTR const epd_init_3 Gdew042t2Grays::epd_soft_start={
0x06,{0x17,0x17,0x17},3
};

// 0xbf, 0x0d seems appropiate for 4 grays mode
DRAM_ATTR const epd_init_2 Gdew042t2Grays::epd_panel_setting={
0x00,{0xbf, 0x0d}, 2
};

DRAM_ATTR const epd_init_1 Gdew042t2Grays::epd_pll={
0x30,{0x3c},1
};

DRAM_ATTR const epd_init_4 Gdew042t2Grays::epd_resolution={
0x61,{
  0x01, 0x90,
  0x01, 0x2c
},4};

// Constructor
Gdew042t2Grays::Gdew042t2Grays(EpdSpi& dio): 
  Adafruit_GFX(GDEW042T2_WIDTH, GDEW042T2_HEIGHT),
  Epd(GDEW042T2_WIDTH, GDEW042T2_HEIGHT), IO(dio)
{
  printf("Gdew042t2Grays() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW042T2_WIDTH, GDEW042T2_HEIGHT);  
}

void Gdew042t2Grays::initFullUpdate(){
    IO.cmd(lut_vcom11.cmd);
    IO.data(lut_vcom11.data,lut_vcom11.databytes);

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

/**
 * @brief Partial update is NOT supported in 4 Grays mode
 * 
 */
void Gdew042t2Grays::initPartialUpdate(){

  // LUT Tables for partial update. Send them directly in 42 bytes chunks. In total 210 bytes
    IO.cmd(lut_partial.cmd);
    IO.data(lut_partial.data,lut_partial.databytes);
 }

//Initialize the display
void Gdew042t2Grays::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew042t2Grays::init(%d) and reset EPD\n", debug);
    //Initialize the Epaper and reset it
    IO.init(4, debug); // 4MHz frequency, debug

    //Reset the display
    IO.reset(20);
    fillScreen(EPD_WHITE);
}

void Gdew042t2Grays::fillScreen(uint16_t color)
{

  for (uint32_t y = 0; y < GDEW042T2_HEIGHT; y++)
  {
    for (uint32_t x = 0; x < GDEW042T2_WIDTH; x++)
    {
    uint8_t *buf_ptr = &_buffer[y * GDEW042T2_WIDTH / 2 + x / 2];

    if (x % 2) {
      *buf_ptr = (*buf_ptr & 0x0F) | (color & 0xF0);
    } else {
      *buf_ptr = (*buf_ptr & 0xF0) | (color >> 4);
    }
    
      if (x % 8 == 0)
        {
          #if defined CONFIG_IDF_TARGET_ESP32
          rtc_wdt_feed();
          #endif
          vTaskDelay(pdMS_TO_TICKS(2));
        }
    }
  }

  if (debug_enabled) printf("fillScreen(%d) _buffer len:%d\n", color,sizeof(_buffer));
}

void Gdew042t2Grays::_wakeUp(){
  IO.reset(10);

  IO.cmd(epd_wakeup_power.cmd);
  for (int i=0;i<epd_wakeup_power.databytes;++i) {
    IO.data(epd_wakeup_power.data[i]);
  }
 
  IO.cmd(epd_soft_start.cmd);
  for (int i=0;i<epd_soft_start.databytes;++i) {
    IO.data(epd_soft_start.data[i]);
  }
  IO.cmd(0x04);
  vTaskDelay(2 / portTICK_PERIOD_MS);

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

  IO.cmd(0x82);    // vcom_DC setting
  IO.data(0x12);   // -0.1 + 18 * -0.05 = -1.0V from OTP, slightly better

  IO.cmd(0x50);    // VCOM AND DATA INTERVAL SETTING
  IO.data(0x97);

  _waitBusy("epd_wakeup_power");
  initFullUpdate();
}


/**** Color display description
      white  gray1  gray2  black
0x10|  01     01     00     00
0x13|  01     00     01     00
****************/
void Gdew042t2Grays::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
  
  uint32_t i,j, bufindex;
  uint8_t temp1,temp2,temp3;
  
  IO.cmd(0x10); //1st buffer: 2 grays

  for(i=0;i<GDEW042T2_BUFFER_SIZE/4;i++)	               //400*300
		{ 
			temp3=0;
      for(j=0;j<4;j++)	
			{
				temp1 = _buffer[i*4+j];
				temp2 = temp1&0xF0 ;
				if(temp2 == 0xF0)
					temp3 |= 0x01;//white
				else if(temp2 == 0x00)
					temp3 |= 0x00;  //black
				else if((temp2>0xA0)&&(temp2<0xF0)) 
					temp3 |= 0x01;  //gray1
				else 
					temp3 |= 0x00; //gray2
				temp3 <<= 1;	
				temp1 <<= 4;
				temp2 = temp1&0xF0 ;
				if(temp2 == 0xF0)  //white
					temp3 |= 0x01;
				else if(temp2 == 0x00) //black
					temp3 |= 0x00;
				else if((temp2>0xA0)&&(temp2<0xF0))
					temp3 |= 0x01; //gray1
				else    
						temp3 |= 0x00;	//gray2	
        if(j!=3)					
			  temp3 <<= 1;	
		 }	
       	IO.data(~temp3);			
		}
  
  IO.cmd(0x13); //2nd buffer: 2 other grays
  for(i=0;i<GDEW042T2_BUFFER_SIZE/4;i++)	               //48000*4   800*480
		{ 
			temp3=0;
      for(j=0;j<4;j++)	
			{
        bufindex = i*4+j;
				temp1 = _buffer[bufindex];
        //printf("%x ",temp1);
				temp2 = temp1&0xF0 ;

				if(temp2 == 0xF0) {
					temp3 |= 0x01;//white
          //printf("W ");
          }
				else if(temp2 == 0x00)
					temp3 |= 0x00;  //black
				else if((temp2>0xA0)&&(temp2<0xF0)) 
					temp3 |= 0x00;  //gray1
				else {
					temp3 |= 0x01; //gray2
          //printf("G2 ");
          }
				temp3 <<= 1;	
				temp1 <<= 4;
				temp2 = temp1&0xF0 ;
				if(temp2 == 0xF0)  //white
					temp3 |= 0x01;
				else if(temp2 == 0x00) //black
					temp3 |= 0x00;
				else if((temp2>0xA0)&&(temp2<0xF0)) 
					temp3 |= 0x00;//gray1
				else    
						temp3 |= 0x01;	//gray2
        if(j!=3)				
			  temp3 <<= 1;				
			
		 }
       	IO.data(~temp3);
        //printf("%x ", temp3);
		}

  uint64_t endTime = esp_timer_get_time();
  IO.cmd(0x12);
  _waitBusy("update");
  uint64_t powerOnTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);

  _sleep();
}

uint16_t Gdew042t2Grays::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
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

void Gdew042t2Grays::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
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

void Gdew042t2Grays::_waitBusy(const char* message){
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

void Gdew042t2Grays::_sleep(){
  IO.cmd(0x50); // border floating
  IO.cmd(0x17);
  IO.data(0x02);// power off
  _waitBusy("power_off");
  IO.cmd(0x07);
  IO.data(0xA5);// power off
}

void Gdew042t2Grays::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
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

/**
 * @brief  Main issue is that Adafruit is RGB565 (16 bit per pixel, not 4-bit like EPDiy)
 *         But that should not be a problem, provided we keep our own _buffer
 * @param x 
 * @param y 
 * @param color 
 */
void Gdew042t2Grays::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Check rotation, move pixel around if necessary
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
  
  uint8_t *buf_ptr = &_buffer[y * GDEW042T2_WIDTH / 2 + x / 2];

  if (x % 2) {
    *buf_ptr = (*buf_ptr & 0x0F) | (color & 0xF0);
  } else {
    *buf_ptr = (*buf_ptr & 0xF0) | (color >> 4);
  }
}
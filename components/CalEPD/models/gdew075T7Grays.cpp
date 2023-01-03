#include "gdew075T7Grays.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Important: I suspect this way of rendering grays is not valid
//            Please check the 04t2Grays and submit a PR to fix this if you can
// Partial Update Delay, may have an influence on degradation
#define GDEW075T7_PU_DELAY 100
#define T1 0x0A

// Grays Waveform
DRAM_ATTR const epd_init_42 Gdew075T7Grays::lut_vcom = {
    0x20, {
      0x00	,T1	,0x00	,0x00	,0x00	,0x01,
0x60	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x00	,0x00	,0x00	,0x01,
0x00	,0x13	,0x0A	,0x01	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00}, 42};

DRAM_ATTR const epd_init_42 Gdew075T7Grays::lut_ww = {
    0x21, {
           0x40	,T1	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x10	,0x14	,0x0A	,0x00	,0x00	,0x01,
0xA0	,0x13	,0x01	,0x00	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T7Grays::lut_bw = {
    0x22, {
           0x40	,T1	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x99	,0x0C	,0x01	,0x03	,0x04	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

DRAM_ATTR const epd_init_42 Gdew075T7Grays::lut_wb = {
    0x23, {
          0x40	,T1	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x00	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x99	,0x0B	,0x04	,0x04	,0x01	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00
          },
    42};

DRAM_ATTR const epd_init_42 Gdew075T7Grays::lut_bb = {
    0x24, {//R24H	b
           0x80	,T1	,0x00	,0x00	,0x00	,0x01,
0x90	,0x14	,0x14	,0x00	,0x00	,0x01,
0x20	,0x14	,0x0A	,0x00	,0x00	,0x01,
0x50	,0x13	,0x01	,0x00	,0x00	,0x01,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00,
0x00	,0x00	,0x00	,0x00	,0x00	,0x00},
    42};

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V
DRAM_ATTR const epd_power_4 Gdew075T7Grays::epd_wakeup_power = {
    0x01, {0x07, 0x17, 0x3f, 0x3f}, 4};

DRAM_ATTR const epd_init_4 Gdew075T7Grays::epd_resolution = {
    0x61, {GDEW075T7_WIDTH / 256, //source 800
           GDEW075T7_WIDTH % 256,
           GDEW075T7_HEIGHT / 256, //gate 480
           GDEW075T7_HEIGHT % 256},
    4};

// Constructor
Gdew075T7Grays::Gdew075T7Grays(EpdSpi &dio) : Adafruit_GFX(GDEW075T7_WIDTH, GDEW075T7_HEIGHT),
                                    Epd(GDEW075T7_WIDTH, GDEW075T7_HEIGHT), IO(dio)
{
  printf("Gdew075T7Grays() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GDEW075T7_WIDTH, GDEW075T7_HEIGHT, (int) GDEW075T7_BUFFER_SIZE);
  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
  printf("Total PSRAM allocated: %d Free: %d\n", info.total_allocated_bytes, info.total_free_bytes);
}


//Initialize the display
void Gdew075T7Grays::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("Gdew075T7Grays::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, false);
  fillScreen(EPD_WHITE);
  _wakeUp();
}

/********Color display description
      white  gray1  gray2  black
0x10|  01     01     00     00
0x13|  01     00     01     00
****************/
void Gdew075T7Grays::fillScreen(uint16_t color)
{
 
  for (uint32_t x = 0; x < GDEW075T7_BUFFER_SIZE; x++)
  {
    _buffer[x] = color;
    if (x % 8 == 0)
        {
          #if defined CONFIG_IDF_TARGET_ESP32 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
          rtc_wdt_feed();
          #endif
          vTaskDelay(pdMS_TO_TICKS(2));
        }
  }
}

void Gdew075T7Grays::_wakeUp()
{
  IO.reset(10);
  //IMPORTANT: Some EPD controllers like to receive data byte per byte
  //So this won't work:
  //IO.data(epd_wakeup_power.data,epd_wakeup_power.databytes);

  IO.cmd(epd_wakeup_power.cmd);
  for (int i = 0; i < epd_wakeup_power.databytes; ++i)
  {
    IO.data(epd_wakeup_power.data[i]);
  }

  IO.cmd(0x04);
  _waitBusy("_wakeUp power on");

  IO.cmd(0x00);	//PANNEL SETTING
  IO.data(0xBF);

  IO.cmd(0x30);  // PLL
  IO.data(0x06);

  // Resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i = 0; i < epd_resolution.databytes; ++i)
  {
    IO.data(epd_resolution.data[i]);
  }
  IO.cmd(0x15);  // SPI Setting
  IO.data(0x00);

  IO.cmd(0x60);  // TCON SETTING
  IO.data(0x22);

  IO.cmd(0x82);  //vcom_DC setting
  IO.data(0x12);

  IO.cmd(0x50);  //VCOM AND DATA INTERVAL SETTING
  IO.data(0x10); //10:KW(0--1)  21:KW(1--0)
  IO.data(0x07);
}

void Gdew075T7Grays::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;

   _wakeUp();
  
  printf("Sending a %d bytes buffer via SPI\n", (int) GDEW075T7_BUFFER_SIZE);
  uint32_t i,j, bufindex;
  uint8_t temp1,temp2,temp3;
  
  IO.cmd(0x10); //1st buffer: 2 grays

  for(i=0;i<48000;i++)	               //48000*4  800*480
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
  for(i=0;i<48000;i++)	               //48000*4   800*480
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

  sendLuts();

  IO.cmd(0x12);
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  _sleep();
}

void Gdew075T7Grays::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  printf("updateWindow: There is no partial update using the Gdew075T7GraysGrays class\n");
}

void Gdew075T7Grays::_waitBusy(const char *message)
{
  if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1)
  {
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1)
      break;
    vTaskDelay(1);
    if (esp_timer_get_time() - time_since_boot > 2000000)
    {
      if (debug_enabled)
        ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew075T7Grays::_sleep()
{
  IO.cmd(0x02);
  _waitBusy("power_off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xA5);
}

void Gdew075T7Grays::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = GDEW075T7_WIDTH - x - w - 1;
    break;
  case 2:
    x = GDEW075T7_WIDTH - x - w - 1;
    y = GDEW075T7_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = GDEW075T7_HEIGHT - y - h - 1;
    break;
  }
}

void Gdew075T7Grays::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = GDEW075T7_WIDTH - x - 1;
    break;
  case 2:
    x = GDEW075T7_WIDTH - x - 1;
    y = GDEW075T7_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = GDEW075T7_HEIGHT - y - 1;
    break;
  }

  uint8_t *buf_ptr = &_buffer[y * GDEW075T7_WIDTH / 2 + x / 2];
  if (x % 2) {
    *buf_ptr = (*buf_ptr & 0x0F) | (color & 0xF0);
  } else {
    *buf_ptr = (*buf_ptr & 0xF0) | (color >> 4);
  }
}

void Gdew075T7Grays::fillRawBufferPos(uint32_t index, uint8_t value) {
  _buffer[index] = value;
}

void Gdew075T7Grays::fillRawBufferImage(uint8_t *image, uint32_t size) {
  for (int i=0; i<size; ++i) {
      _buffer[i] = image[i];
      if (image[i]!=0) {
        printf("%x ", image[i]);
      }
   }
}

void Gdew075T7Grays::sendLuts() {
// LUT for grays
  IO.cmd(0x20);
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_vcom.data[i]);
  }
  IO.cmd(0x21); //red not use
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_ww.data[i]);
  }
  IO.cmd(0x22); //bw r
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_bw.data[i]);
  }
  IO.cmd(0x23); //wb w
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_wb.data[i]);
  }
  IO.cmd(0x24); //bb b
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_bb.data[i]);
  }
  IO.cmd(0x25); //vcom
  for (int i = 0; i < 42; ++i) {
  IO.data(lut_ww.data[i]);
  //printf("%x ",lut_ww.data[i]);
  }
  printf("\n");
}

void Gdew075T7Grays::test4bit() {
  		unsigned int i,r,t,y;
		
		IO.cmd(0x10);	  	
		for(y=0;y<12000;y++)	     
		{
			IO.data(0xff);  //white
		}  
		for(t=0;t<12000;t++)	     
		{
			IO.data(0xff);  //gray1
		}  
		for(r=0;r<12000;r++)	     
		{
			IO.data(0x00);  //gray2
		}  
		for(i=0;i<12000;i++)	     
		{
			IO.data(0x00);  //black
		}  

		IO.cmd(0x13);
		for(i=0;i<12000;i++)	     
		{
			IO.data(0xff);  //white
		}  
		for(r=0;r<12000;r++)	     
		{
			IO.data(0x00); //gray1
		}  
		for(t=0;t<12000;t++)	     
		{
			IO.data(0xff);  //gray2
		}  
		for(y=0;y<12000;y++)	     
		{
			IO.data(0x00); //black
		}  
}
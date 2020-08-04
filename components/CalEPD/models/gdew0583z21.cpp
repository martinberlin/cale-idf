//Controller: IL0371 (3 colors) http://www.e-paper-display.com/download_detail/downloadsId%3d536.html
#include "gdew0583z21.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// CMD, DATA, Databytes * Optional we are going to use sizeof(data)
DRAM_ATTR const epd_init_2 Gdew0583z21::epd_wakeup_power={
0x01,{
  0x37,
  0x00},2
};

DRAM_ATTR const epd_init_2 Gdew0583z21::epd_panel_setting={
0x00,{
  0xCF,
  0x08},1
};

DRAM_ATTR const epd_init_4 Gdew0583z21::epd_resolution={
0x61,{
  0x02, //source 600
  0x58,
  0x01, //gate 448
  0xc0
},4};

// Constructor
Gdew0583z21::Gdew0583z21(EpdSpi& dio): 
  Adafruit_GFX(GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT),
  Epd(GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT), IO(dio)
{
  printf("Gdew0583z21() constructor injects IO and extends Adafruit_GFX(%d,%d)\n",
  GDEW0583Z21_WIDTH, GDEW0583Z21_HEIGHT);  
}

//Initialize the display
void Gdew0583z21::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Gdew0583z21::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug
    IO.init(4, true);
    fillScreen(EPD_WHITE);
}

void Gdew0583z21::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == EPD_WHITE);
  else if (color == EPD_BLACK) black = 0xFF;
  else if (color == EPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = black;
    _red_buffer[x] = red;
  }
}

void Gdew0583z21::_wakeUp(){
  IO.reset(10);
  IO.cmd(epd_wakeup_power.cmd);
 
  for (int i=0;i<sizeof(epd_wakeup_power.data);++i) {
    IO.data(epd_wakeup_power.data[i]);
  }
  printf("Panel setting\n");
  IO.cmd(epd_panel_setting.cmd);
  IO.data(epd_panel_setting.data[0]); //0x0f KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f

  printf("PLL\n");
  IO.cmd(0x30);
  IO.data(0x3a);

  printf("VCOM\n");
  IO.cmd(0x82);
  IO.data(0x28);
  
  printf("boost\n");
  IO.cmd(0x06);
	IO.data(0xc7);	   	
	IO.data(0xcc);
	IO.data(0x15);

	IO.cmd(0X50);			//VCOM AND DATA INTERVAL SETTING
	IO.data(0x77);

	IO.cmd(0X60);			//TCON SETTING
	IO.data(0x22);

	IO.cmd(0X65);			//FLASH CONTROL
	IO.data(0x00);
  
  printf("Resolution setting\n");
  IO.cmd(epd_resolution.cmd);
  for (int i=0;i<sizeof(epd_resolution.data);++i) {
    IO.data(epd_resolution.data[i]);
  }
  printf("Flash mode\n");
  IO.cmd(0xe5);
	IO.data(0x03);

  // Power it on
  IO.cmd(0x04);
  _waitBusy("Power on");

}

void Gdew0583z21::update()
{
  uint64_t startTime = esp_timer_get_time();
  _using_partial_mode = false;
  _wakeUp();
  // IN GD example says bufferSize is 38880 (?)
  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n",sizeof(_buffer));

  for (uint32_t i = 0; i < sizeof(_buffer); i++)
  {
    _send8pixel(_buffer[i], _red_buffer[i]);
    
    if (i%2000==0) {
       rtc_wdt_feed();
       vTaskDelay(pdMS_TO_TICKS(10));
       if (debug_enabled) printf("%d ",i);
    }
  
  }
  IO.cmd(0x12);
  
  uint64_t endTime = esp_timer_get_time();
  _waitBusy("update");
  uint64_t updateTime = esp_timer_get_time();
  
  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);

  //vTaskDelay(pdMS_TO_TICKS(1000));
  _sleep();
}

void Gdew0583z21::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (true){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>2000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Gdew0583z21::_sleep(){
  // Flash sleep  
  IO.cmd(0x02);
  _waitBusy("Power Off");
  IO.cmd(0x07); // Deep sleep
  IO.data(0xa5);
}

void Gdew0583z21::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = GDEW0583Z21_WIDTH - x - w - 1;
      break;
    case 2:
      x = GDEW0583Z21_WIDTH - x - w - 1;
      y = GDEW0583Z21_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = GDEW0583Z21_HEIGHT - y - h - 1;
      break;
  }
}

void Gdew0583z21::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GDEW0583Z21_WIDTH - x - 1;
      break;
    case 2:
      x = GDEW0583Z21_WIDTH - x - 1;
      y = GDEW0583Z21_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GDEW0583Z21_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GDEW0583Z21_WIDTH / 8;

  // This formulas are from gxEPD that apparently got the color right:
  _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8)))); // white
  if (color == EPD_WHITE) return;
  else if (color == EPD_BLACK) _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else if (color == EPD_RED) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
  else
  {
    if ((color & 0xF100) > (0xF100 / 2)) _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2)
    {
      _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
  }
}

void Gdew0583z21::_send8pixel(uint8_t data, uint8_t red)
{ 
  for (uint8_t j = 0; j < 8; j++)
  {
    //0x04 RED
    uint8_t tr = red & 0x80 ? 0x04 : 0x03; 
    tr <<= 4;
    red <<= 1;
    tr |= red & 0x80 ? 0x04 : 0x03;
    red <<= 1;
    
    //0x03 WHITE 0x00 BLACK
    uint8_t t = data & 0x80 ? 0x00: tr;
    t <<= 4;
    data <<= 1;
    j++;
    t |= data & 0x80 ? 0x00 : tr;
    data <<= 1;

    IO.dataBuffer(t);
  } 
  
}

/**
 * Example from Good display
 * Does not work / is just an internal private reference
 */
void Gdew0583z21::PIC_display(const unsigned char* picData)
{
  uint32_t i,j;
	uint8_t temp1,temp2,temp3;
		//EPD_W21_WriteCMD(0x10);	     //start to transport picture

		for(i=0;i<67200;i++)	     //2bit for a pixels(old is 4bit for a pixels)   
		{   
			temp1 = *picData;
			picData++;
			for(j=0;j<2;j++)         //2bit to 4bit
			{
				temp2 = temp1&0xc0 ;   //Analysis the first 2bit
				if(temp2 == 0xc0)
					temp3 = 0x00; 			 //black(2bit to 4bit)
				else if(temp2 == 0x00)
					temp3 = 0x03;        //white(2bit to 4bit)
				else
					temp3 = 0x04;        //red(2bit to 4bit)
					
				temp3 <<=4;            //move to the Hight 4bit
				temp1 <<=2;            //move 2bit	
				temp2 = temp1&0xc0 ;   //Analysis the second 2bit
				if(temp2 == 0xc0)
					temp3 |= 0x00;       //black(2bit to 4bit),Data consolidation
				else if(temp2 == 0x00)
					temp3 |= 0x03;       //white(2bit to 4bit),Data consolidation
				else
					temp3 |= 0x04;       //red(2bit to 4bit),Data consolidation
				
				temp1 <<=2;            //move 2bit£¬turn the next 2bit
				
				//EPD_W21_WriteDATA(temp3); //write a byte,Contains two 4bit pixels	
			}
		}
}
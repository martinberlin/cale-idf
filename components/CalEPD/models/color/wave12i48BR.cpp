// Black & Red version for Waveshare 12.48 inches epaper
#include "wave12i48BR.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f
DRAM_ATTR const epd_init_1 Wave12I48RB::epd_panel_setting_full={
0x00,{0x2f},1
};

DRAM_ATTR const epd_init_4 Wave12I48RB::epd_resolution_m1s2={
0x61,{0x02,0x88,0x01,0xEC},4};

DRAM_ATTR const epd_init_4 Wave12I48RB::epd_resolution_m2s1={
0x61,{0x02,0x90,0x01,0xEC},4};

// LUT Tables
unsigned char Wave12I48RB::lut_vcom1[] = {
    0x00,	0x10,	0x10,	0x01,	0x08,	0x01,
    0x00,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x00,	0x08,	0x01,	0x08,	0x01,	0x06,
    0x00,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x06,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x01,
    0x00,	0x04,	0x05,	0x08,	0x08,	0x01,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
};
unsigned char Wave12I48RB::lut_ww1[] = {
    0x91,	0x10,	0x10,	0x01,	0x08,	0x01,
    0x04,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x84,	0x08,	0x01,	0x08,	0x01,	0x06,
    0x80,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x06,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x01,
    0x08,	0x04,	0x05,	0x08,	0x08,	0x01,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
};
unsigned char Wave12I48RB::lut_bw1[] = {
    0xA8,	0x10,	0x10,	0x01,	0x08,	0x01,
    0x84,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x84,	0x08,	0x01,	0x08,	0x01,	0x06,
    0x86,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x8C,	0x05,	0x01,	0x1E,	0x0F,	0x06,
    0x8C,	0x05,	0x01,	0x1E,	0x0F,	0x01,
    0xF0,	0x04,	0x05,	0x08,	0x08,	0x01,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
};
unsigned char Wave12I48RB::lut_wb1[] = {
    0x91,	0x10,	0x10,	0x01,	0x08,	0x01,
    0x04,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x84,	0x08,	0x01,	0x08,	0x01,	0x06,
    0x80,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x06,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x01,
    0x08,	0x04,	0x05,	0x08,	0x08,	0x01,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
};
unsigned char Wave12I48RB::lut_bb1[] = {
    0x92,	0x10,	0x10,	0x01,	0x08,	0x01,
    0x80,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x84,	0x08,	0x01,	0x08,	0x01,	0x06,
    0x04,	0x06,	0x01,	0x06,	0x01,	0x05,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x06,
    0x00,	0x05,	0x01,	0x1E,	0x0F,	0x01,
    0x01,	0x04,	0x05,	0x08,	0x08,	0x01,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
    0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
};

// Constructor
Wave12I48RB::Wave12I48RB(Epd4Spi& dio): 
  Adafruit_GFX(WAVE12I48_WIDTH, WAVE12I48_HEIGHT),
  Epd(WAVE12I48_WIDTH, WAVE12I48_HEIGHT), IO(dio)
{
  rtc_wdt_feed();
  vTaskDelay(pdMS_TO_TICKS(1));
  printf("Wave12I48RB() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
  WAVE12I48_WIDTH, WAVE12I48_HEIGHT, WAVE12I48_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n",xPortGetFreeHeapSize());
}

void Wave12I48RB::initFullUpdate(){
  printf("Not implemented for this Epd\n");
}

void Wave12I48RB::initPartialUpdate(){
  printf("Not implemented for this Epd\n");
 }

// Initialize the display
void Wave12I48RB::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Wave12I48RB::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug (Increasing up to 6 still works but no noticiable speed change)
    IO.init(4, debug_enabled);

    fillScreen(EPD_WHITE);
    printf("\nAvailable heap after Epd init:%d\n",xPortGetFreeHeapSize());
    //clear(); // No need to do this, but will leave it in the class
}

void Wave12I48RB::fillScreen(uint16_t color)
{
  if (debug_enabled) printf("fillScreen(%x) Buffer size:%d\n",color,sizeof(_buffer));
  uint8_t data = (color == EPD_BLACK) ? WAVE12I48_8PIX_BLACK : WAVE12I48_8PIX_WHITE;
  for (uint32_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }
}

void Wave12I48RB::_powerOn(){
    // Power on
  IO.cmdM1(0x04);
  IO.cmdM2(0x04);
  vTaskDelay(pdMS_TO_TICKS(300));
  IO.cmdM1S1M2S2(0x12);
  _waitBusyM1("display refresh");
  _waitBusyM2("display refresh");
}

void Wave12I48RB::_setLut(){
  printf("\nSending LUT init tables\n");
  uint8_t count = 0;
  IO.cmdM1S1M2S2(0x20);							//vcom
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_vcom1[count]);
  }

  IO.cmdM1S1M2S2(0x21);							//red not use
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_ww1[count]);
  }

  IO.cmdM1S1M2S2(0x22);							//bw r
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_bw1[count]);   // bw=r
  }

  IO.cmdM1S1M2S2(0x23);							//wb w
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_wb1[count]);   // wb=w
  }

  IO.cmdM1S1M2S2(0x24);							//bb b
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_bb1[count]);   // bb=b
  }

  IO.cmdM1S1M2S2(0x25);							//bb b
  for(count=0; count<60; count++) {
      IO.dataM1S1M2S2(lut_ww1[count]);   // bb=b
  }
}

void Wave12I48RB::_wakeUp(){
  IO.reset(200);
  // Panel setting
  printf("Panel setting\n");
  IO.cmdM1(epd_panel_setting_full.cmd);
  IO.dataM1(epd_panel_setting_full.data[0]);
  IO.cmdS1(epd_panel_setting_full.cmd);
  IO.dataS1(epd_panel_setting_full.data[0]);
  IO.cmdM2(epd_panel_setting_full.cmd);
  IO.dataM2(0x23);
  IO.cmdS2(epd_panel_setting_full.cmd);
  IO.dataS2(0x23);

  printf("Power setting\n");
  // POWER SETTING
  IO.cmdM1(0x01);
  IO.dataM1(0x07);
  IO.dataM1(0x17);	//VGH=20V,VGL=-20V
  IO.dataM1(0x3F);	//VDH=15V
  IO.dataM1(0x3F);  //VDL=-15V
  IO.dataM1(0x0d);
  IO.cmdM2(0x01);
  IO.dataM2(0x07);
  IO.dataM2(0x17);	//VGH=20V,VGL=-20V
  IO.dataM2(0x3F);	//VDH=15V
  IO.dataM2(0x3F);  //VDL=-15V
  IO.dataM2(0x0d);

  // booster soft start
  IO.cmdM1(0x06);
  IO.dataM1(0x17);	//A
  IO.dataM1(0x17);	//B
  IO.dataM1(0x39);	//C
  IO.dataM1(0x17);
  IO.cmdM2(0x06);
  IO.dataM2(0x17);
  IO.dataM2(0x17);
  IO.dataM2(0x39);
  IO.dataM2(0x17);

  printf("\nResolution setting\n");
  IO.cmdM1(epd_resolution_m1s2.cmd);
  for (int i=0;i<epd_resolution_m1s2.databytes;++i) {
    IO.dataM1(epd_resolution_m1s2.data[i]);
  }
  IO.cmdS1(epd_resolution_m2s1.cmd);
  for (int i=0;i<epd_resolution_m2s1.databytes;++i) {
    IO.dataS1(epd_resolution_m2s1.data[i]);
  }
  IO.cmdM2(epd_resolution_m2s1.cmd);
  for (int i=0;i<epd_resolution_m2s1.databytes;++i) {
    IO.dataM2(epd_resolution_m2s1.data[i]);
  }
  IO.cmdS2(epd_resolution_m1s2.cmd);
  for (int i=0;i<epd_resolution_m1s2.databytes;++i) {
    IO.dataS2(epd_resolution_m1s2.data[i]);
  }
  
  printf("\nDu SPI\n");
  IO.cmdM1S1M2S2(0x15);  // DUSPI
  IO.dataM1S1M2S2(0x20);

  printf("\nPLL\n");
  IO.cmdM1S1M2S2(0x30);  // PLL
  IO.dataM1S1M2S2(0x08);

  printf("\nVcom and data interval\n");
  IO.cmdM1S1M2S2(0x50);  //Vcom and data interval setting
  IO.dataM1S1M2S2(0x31); //Border KW
  IO.dataM1S1M2S2(0x07);

  printf("\nTCON\n");
  IO.cmdM1S1M2S2(0x60);  //TCON
  IO.dataM1S1M2S2(0x22);
  
  IO.cmdM1(0xE0);        // Power setting
  IO.dataM1(0x01);
  IO.cmdM2(0xE0);
  IO.dataM2(0x01);

  IO.cmdM1S1M2S2(0xE3);
  IO.dataM1S1M2S2(0x00);

  IO.cmdM1(0x82);
  IO.dataM1(0x1c);
  IO.cmdM2(0x82);
  IO.dataM2(0x1c);
  
  // Acording to Waveshare/GoodDisplay code this needs LUT Tables
  // Comment next line if it does not work. 
  _setLut();
}

void Wave12I48RB::update()
{
  uint64_t startTime = esp_timer_get_time();
  _wakeUp();
  
  printf("Sending a buffer[%d] via SPI\n",sizeof(_buffer));
  uint32_t i = 0;

  IO.cmdM1S1M2S2(0x13);

  /*
   DISPLAYS:
  __________
  | S2 | M2 |
  -----------
  | M1 | S1 |
  -----------
  */
  uint8_t x1buf[81];
  uint8_t x2buf[82];

  // Optimized to send in 81/82 byte chuncks (v2 after our conversation with Samuel)
  for(uint16_t y =  1; y <= WAVE12I48_HEIGHT; y++) {
        for(uint16_t x = 1; x <= WAVE12I48_WIDTH/8; x++) {
          uint8_t data = i < sizeof(_buffer) ? _buffer[i] : 0x00;

        if (y <= 492) {  // S2 & M2 area
          if (x <= 81) { // 648/8 -> S2
            x1buf[x-1] = data;
          } else {       // M2
            x2buf[x-82] = data;
          }

          if (x==WAVE12I48_WIDTH/8) {  // Send the complete X line for S2 & M2
                IO.dataS2(x1buf,sizeof(x1buf));
                IO.dataM2(x2buf,sizeof(x2buf));
          }

        } else {         // M1 & S1
          if (x <= 81) { // 648/8 -> M1
            x1buf[x-1] = data;
          } else {       // S1
            x2buf[x-82] = data;
          }

          if (x==WAVE12I48_WIDTH/8) { // Send the complete X line for M1 & S1
              IO.dataM1(x1buf,sizeof(x1buf));
              IO.dataS1(x2buf,sizeof(x2buf));
          }
        }
          ++i;
        }
  }
  uint64_t endTime = esp_timer_get_time();
  _powerOn();
  uint64_t powerOnTime = esp_timer_get_time();
  printf("\nAvailable heap after Epd update: %d bytes\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu _powerOn\n%llu total time in millis\n",
  xPortGetFreeHeapSize(), (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);
}

uint16_t Wave12I48RB::_setPartialRamArea(uint16_t, uint16_t, uint16_t, uint16_t){
  printf("_setPartialRamArea not implemented in this Epd\n");
  return 0;
}

void Wave12I48RB::_waitBusy(const char* message){
  _waitBusyM1(message);
}

void Wave12I48RB::_waitBusyM1(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusyM1 for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_SPI_M1_BUSY) == 1) break;
    IO.cmdM1(0x71);
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>WAVE_BUSY_TIMEOUT)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(200));
}

void Wave12I48RB::_waitBusyM2(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusyM2 for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_SPI_M2_BUSY) == 1) break;
    IO.cmdM1(0x71);
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>WAVE_BUSY_TIMEOUT)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(200));
}

void Wave12I48RB::_waitBusyS1(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusyS1 for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_SPI_S1_BUSY) == 1) break;
    IO.cmdM1(0x71);
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>WAVE_BUSY_TIMEOUT)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(200));
}

void Wave12I48RB::_waitBusyS2(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusyS2 for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_SPI_S2_BUSY) == 1) break;
    IO.cmdM1(0x71);
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>WAVE_BUSY_TIMEOUT)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(200));
}

void Wave12I48RB::_sleep(){
  IO.cmdM1S1M2S2(0x02); // power off
  vTaskDelay(pdMS_TO_TICKS(300));
  IO.cmdM1S1M2S2(0x07); // Deep sleep
  IO.dataM1S1M2S2(0xA5);
  vTaskDelay(pdMS_TO_TICKS(300));
}

void Wave12I48RB::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = WAVE12I48_WIDTH - x - w - 1;
      break;
    case 2:
      x = WAVE12I48_WIDTH - x - w - 1;
      y = WAVE12I48_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = WAVE12I48_HEIGHT - y - h - 1;
      break;
  }
}

void Wave12I48RB::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = WAVE12I48_WIDTH - x - 1;
      break;
    case 2:
      x = WAVE12I48_WIDTH - x - 1;
      y = WAVE12I48_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = WAVE12I48_HEIGHT - y - 1;
      break;
  }
  uint32_t i = x / 8 + y * WAVE12I48_WIDTH / 8;

  if (color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
}

void Wave12I48RB::clear(){
  printf("EPD_12in48_Clear start ...\n");
    //M1 part 648*492
    IO.cmdM1(0x10);
    for(uint16_t y =  492; y < 984; y++)
        for(uint16_t x = 0; x < 81; x++) {
            IO.dataM1(0xff);
        }
    IO.cmdM1(0x13);
    for(uint16_t y = 492; y < 984; y++)
        for(uint16_t x = 0; x < 81; x++) {
            IO.dataM1(0xff);
        }

    //S1 part 656*492
    IO.cmdS1(0x10);
    for(uint16_t y = 492; y < 984; y++)
        for(uint16_t x = 81; x < 163; x++) {
            IO.dataS1(0xff);
        }
    IO.cmdS1(0x13);
    for(uint16_t y = 492; y < 984; y++)
        for(uint16_t x = 81; x < 163; x++) {
            IO.dataS1(0xff);
        }

    //M2 part 656*492
    IO.cmdM2(0x10);
    for(uint16_t y = 0; y < 492; y++)
        for(uint16_t x = 81; x < 163; x++) {
            IO.dataM2(0xff);
        }
    IO.cmdM2(0x13);
    for(uint16_t y = 0; y < 492; y++)
        for(uint16_t x = 81; x < 163; x++) {
            IO.dataM2(0xff);
        }

    //S2 part 648*492
    IO.cmdS2(0x10);
    for(uint16_t y = 0; y < 492; y++)
        for(uint16_t x = 0; x < 81; x++) {
            IO.dataS2(0xff);
        }
    IO.cmdS2(0x13);
    for(uint16_t y = 0; y < 492; y++)
        for(uint16_t x = 0; x < 81; x++) {
            IO.dataS2(0xff);
        }
}

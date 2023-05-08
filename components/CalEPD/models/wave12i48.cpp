// WARNING: This epaper requires an ESP32 with PSRAM (Like esp32-wrover)
#include "wave12i48.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
DRAM_ATTR const epd_init_1 Wave12I48::epd_panel_setting_full={
0x00,{0x1f},1
};

DRAM_ATTR const epd_init_4 Wave12I48::epd_resolution_m1s2={
0x61,{0x02,0x88,0x01,0xEC},4};

DRAM_ATTR const epd_init_4 Wave12I48::epd_resolution_m2s1={
0x61,{0x02,0x90,0x01,0xEC},4};

// Constructor
Wave12I48::Wave12I48(Epd4Spi& dio): 
  Adafruit_GFX(WAVE12I48_WIDTH, WAVE12I48_HEIGHT),
  Epd(WAVE12I48_WIDTH, WAVE12I48_HEIGHT), IO(dio)
{
  printf("Wave12I48() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\nNOTE: Requires external RAM\n",
  WAVE12I48_WIDTH, WAVE12I48_HEIGHT, (int) WAVE12I48_BUFFER_SIZE);
}

// Initialize the display
void Wave12I48::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Wave12I48::init(debug:%d)\n", debug);
    //Initialize SPI at 4MHz frequency. true for debug (Increasing up to 6 still works but no noticiable speed change)
    IO.init(5, false);

    fillScreen(EPD_WHITE);
    //printf("\nAvailable heap after Epd init:%d\n", (int) xPortGetFreeHeapSize());
}

void Wave12I48::fillScreen(uint16_t color)
{
  if (debug_enabled) printf("fillScreen(%x) Buffer size:%d\n", color, (int)WAVE12I48_BUFFER_SIZE);
  uint8_t data = (color == EPD_BLACK) ? WAVE12I48_8PIX_BLACK : WAVE12I48_8PIX_WHITE;
  for (uint32_t x = 0; x < WAVE12I48_BUFFER_SIZE; x++)
  {
    _buffer[x] = data;
  }
}

void Wave12I48::_powerOn(){
    // Power on
  IO.cmdM1(0x04);
  IO.cmdM2(0x04);
  vTaskDelay(pdMS_TO_TICKS(300));
  IO.cmdM1S1M2S2(0x12);
  _waitBusyM1("display refresh");
  _waitBusyM2("display refresh");
}

void Wave12I48::_wakeUp(){
  IO.reset(200);
  // Panel setting
  IO.cmdM1(epd_panel_setting_full.cmd);
  IO.dataM1(epd_panel_setting_full.data[0]);
  IO.cmdS1(epd_panel_setting_full.cmd);
  IO.dataS1(epd_panel_setting_full.data[0]);
  IO.cmdM2(epd_panel_setting_full.cmd);
  IO.dataM2(0x13);
  IO.cmdS2(epd_panel_setting_full.cmd);
  IO.dataS2(0x13);

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

  printf("Resolution setting\n");
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
  
  IO.cmdM1S1M2S2(0x15);  // DUSPI
  IO.dataM1S1M2S2(0x20);

  IO.cmdM1S1M2S2(0x50);  //Vcom and data interval setting
  IO.dataM1S1M2S2(0x21); //Border KW
  IO.dataM1S1M2S2(0x07);

  IO.cmdM1S1M2S2(0x60);  //TCON
  IO.dataM1S1M2S2(0x22);

  IO.cmdM1S1M2S2(0xE3);
  IO.dataM1S1M2S2(0x00);

  IO.cmdM1S1M2S2(0xe0);  //Cascade setting
  IO.dataM1S1M2S2(0x03);
    
  IO.cmdM1S1M2S2(0xe5);//Force temperature
  IO.dataM1S1M2S2(0x00);

}

void Wave12I48::update()
{
  uint64_t startTime = esp_timer_get_time();
  _wakeUp();
  
  printf("Sending a buffer[%d] via SPI\n", (int)WAVE12I48_BUFFER_SIZE);
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
          uint8_t data = i < WAVE12I48_BUFFER_SIZE ? _buffer[i] : 0x00;

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
  (int)xPortGetFreeHeapSize(), (endTime-startTime)/1000, (powerOnTime-endTime)/1000, (powerOnTime-startTime)/1000);
}

uint16_t Wave12I48::_setPartialRamArea(uint16_t, uint16_t, uint16_t, uint16_t){
  printf("_setPartialRamArea not implemented in this Epd\n");
  return 0;
}

void Wave12I48::_waitBusy(const char* message){
  _waitBusyM1(message);
}

void Wave12I48::_waitBusyM1(const char* message){
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

void Wave12I48::_waitBusyM2(const char* message){
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

void Wave12I48::_waitBusyS1(const char* message){
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

void Wave12I48::_waitBusyS2(const char* message){
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

void Wave12I48::_sleep(){
  IO.cmdM1S1M2S2(0x02); // power off
  vTaskDelay(pdMS_TO_TICKS(300));
  IO.cmdM1S1M2S2(0x07); // Deep sleep
  IO.dataM1S1M2S2(0xA5);
  vTaskDelay(pdMS_TO_TICKS(300));
}

void Wave12I48::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
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

void Wave12I48::drawPixel(int16_t x, int16_t y, uint16_t color) {
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

void Wave12I48::clear(){
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
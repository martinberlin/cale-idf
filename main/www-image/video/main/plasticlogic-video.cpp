/**
 * plasticlogic.com Demo of slow RAW RGB565 video read from SPIFFs
 * Inspired by this blogpost: https://appelsiini.net/2020/esp32-mjpeg-video-player/
 * 
 * To read about SPIFFs
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include <plasticlogic021.h>
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic021 display(io);

extern "C"
{
   void app_main();
}
// IDF function to emulate Arduino's millis()
uint32_t millis() {
   return esp_timer_get_time()/1000;
}

void app_main(void)
{
   printf("CalEPD version: %s for Plasticlogic.com\nVIDEO demo\n", CALEPD_VERSION);
   
   /** Color constants that the epaper supports:
    EPD_BLACK 0x00
    EPD_DGRAY 0x01
    EPD_LGRAY 0x02
    EPD_WHITE 0x03
    Fill the buffer fast and update
    */
   
  // Initialize display class
  display.init();         // Add init(true) for debug
  display.setFont(&Ubuntu_M16pt8b);
  display.setCursor(2,20);
  display.setTextColor(EPD_WHITE);
  display.clearScreen();
  display.update();
  for (int i=1; i < sizeof(display._buffer); i++) {
    display._buffer[i] = 0x55;
  }
  display.print("This is FB set to 55");
  display.update(EPD_UPD_PART);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  for (int i=1; i < sizeof(display._buffer); i++) {
    display._buffer[i] = 0xAA;
  }
  display.setCursor(2,20);
  display.print("This is FB set to AA");
  display.update(EPD_UPD_PART);
  vTaskDelay(500 / portTICK_PERIOD_MS);

  for (int i=1; i < sizeof(display._buffer); i++) {
    display._buffer[i] = 0x00;
  }
  display.setCursor(2,20);
  display.print("This is FB set to 00");
  display.update(EPD_UPD_PART);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  display.clearScreen();
  display.setTextColor(EPD_DGRAY);
  display.print("This text is BLACK");
  display.update(EPD_UPD_PART);
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Only for parallel epaper displays driven by I2S DataBus (No SPI)
// NOTE: This needs Epdiy component https://github.com/vroland/epdiy
// Run idf.py menuconfig-> Component Config -> E-Paper driver and select:
// Display type: LILIGO 4.7 ED047TC1
// Board: LILIGO T5-4.7 Epaper
// In the same sectiion Component Config -> ESP32 Specifics -> Enable PSRAM
#include "ED047TC1.h"
uint8_t *framebuffer;
Ed047TC1 display;

extern "C"
{
   void app_main();
}

void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void app_main(void)
{
   display.init(true);
   framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
   memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  // Clear all screen to white
   display.clearScreen();

   delay(1000);

  // draw line
  epd_draw_hline(20, 20, ED047TC1_WIDTH - 40, 0x00, framebuffer);
  
   for (int x = 0; x < 200; x++) {

      display.drawPixel(x, 20, 0x00,framebuffer);
      display.drawPixel(x, 40, 0x20,framebuffer);

      display.drawPixel(x, 50, 0x60,framebuffer);
      display.drawPixel(x, 51, 0x60,framebuffer);

      display.drawPixel(x, 60, 0x90,framebuffer);
      display.drawPixel(x, 61, 0x90,framebuffer);
   } 
   //epd_draw_grayscale_image(epd_full_screen(), framebuffer);
   display.update(framebuffer);

   delay(1999);
  // Refresh only this area: 
  Rect_t to_clear = {
      .x = 400,
      .y = 800,
      .width = 100,
      .height = 50,
  };
  display.clearArea(to_clear);

  printf("Simple test: Just doing a bridge class that implements epd_* functions\n");
}

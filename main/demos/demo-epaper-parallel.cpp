#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Needs Epdiy component for paralell epapers driven by I2S Databus driver
// components: Epdiy
// Only for parallel epaper displays driven by I2S DataBus (No SPI)
#include "ED047TC1.h"

Ed047TC1 display;

extern "C"
{
   void app_main();
}

void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void app_main(void)
{
   display.init(true);

  // Clear all screen to white
   display.clearScreen();

   delay(1000);
  // Refresh only this area: 
  Rect_t to_clear = {
      .x = 50,
      .y = 300,
      .width = 200,
      .height = 400,
  };
  display.clearArea(to_clear);

   printf("Simple test: Just doing a bridge class that implements epd_* functions\n");
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Needs Epdiy component for paralell epapers driven by I2S Databus driver
#include "ED047TC1.h"

Ed047TC1 display;

extern "C"
{
   void app_main();
}

void app_main(void)
{
   display.init(true);
   display.clearScreen();

   printf("Simple test: Just doing a bridge class that implements epd_* functions\n");
}

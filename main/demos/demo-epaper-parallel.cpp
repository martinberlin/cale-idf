#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Needs Epdiy component for paralell epapers driven by I2S Databus driver
#include "i2s_data_bus.h"

// I2S data bus by vroland
I2SDataBus io;

extern "C"
{
   void app_main();
}

void app_main(void)
{
   printf("Still not implemented");
}

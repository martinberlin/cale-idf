#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Needs Epdiy component for paralell epapers driven by I2S Databus driver
#include "i2s_data_bus.h"
// Common methods that all I2Sbus epapers should inherit
#include "epd_driver_base.h"

// I2S data bus by vroland
I2SDataBus io; // Needs to be injected in EpdDriver
EpdDriver epaper(io);

extern "C"
{
   void app_main();
}

void app_main(void)
{
   epaper.epd_init();
   epaper.epd_poweron();
   printf("Still not implemented");
}

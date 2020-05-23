
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <epd.h>

extern "C" {
   void app_main();
}

Epd display;

void app_main(void)
{
    printf("Hello world!\n");
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores\n",
            CONFIG_IDF_TARGET,
            chip_info.cores);

    printf("Silicon revision %d, ", chip_info.revision);
    printf("Free heap: %d\n", esp_get_free_heap_size());
       
       display.init(true);
       //display.fillScreen(GxEPD_BLACK);
       //display.update();

       // Do nothing loop
       uint16_t counter = 0;
       while(1) {
	    printf("Loop test %d\n", counter);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ++counter;
       }
    
}

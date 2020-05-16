/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "Arduino.h"

//#include "GxEPD.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include <GxGDEW042T2.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
// FONT used for title / message body
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/FreeMono9pt7b.h>
// SPI interface GPIOs defined in Config.h  
GxIO_Class io(SPI, CONFIG_EINK_SPI_CS, CONFIG_EINK_DC, CONFIG_EINK_RST);

// This line is returning:
// cale.cpp:31: undefined reference to `GxGDEW042T2::GxGDEW042T2(GxIO&, signed char, signed char)
GxEPD_Class display(&io, (int8_t)CONFIG_EINK_RST, (int8_t)CONFIG_EINK_BUSY); // (GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
// Writing it this way: 
// GxEPD_Class display(&io, (int8_t)CONFIG_EINK_RST, (int8_t)CONFIG_EINK_BUSY);
/* 
../components/GxEPD/src/GxGDEW042T2/GxGDEW042T2.h:40:5: note: candidate: 'GxGDEW042T2::GxGDEW042T2(GxIO&, int8_t, int8_t)'
     GxGDEW042T2(GxIO& io, int8_t rst = 9, int8_t busy = 7);
     ^~~~~~~~~~~
../components/GxEPD/src/GxGDEW042T2/GxGDEW042T2.h:40:5: note:   no known conversion for argument 1 from 'GxIO_SPI*' to 'GxIO&'
 */
extern "C" {
   void app_main();
}

void app_main(void)
{
    initArduino();
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Free heap: %d\n", esp_get_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

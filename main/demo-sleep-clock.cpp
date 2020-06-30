#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include <sstream>
// Non-Volatile Storage (NVS) - borrrowed from esp-idf/examples/storage/nvs_rw_value
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
// Epd display class
#include <gdew027w3.h>
#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

/*
 * - - - - - - - - Deepsleep clock example - - - - - - - - - - - - - - - - - - - - 
 * Please note that the intention of this clock is not to be precise. 
 * Just a simple: Sleep every N minutes, increment EPROM variable, refresh epaper.
 * And once a day or every hour, a single HTTP request to sync the hour online. 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
EpdSpi io;
Gdew027w3 display(io);

// Time query: HHmm  -> 0800 (8 AM)
const char* timequery = "http://fs.fasani.de/api/?q=date&timezone=Europe/Berlin&f=Hi";

// Clock will refresh each N minutes
int sleepMinutes = 5;

// Values that will be stored in NVS - defaults should come initially from timequery (external HTTP request)
int8_t nvs_hour = 10;
int8_t nvs_minute = 20;

extern "C"
{
   void app_main();
}

void deepsleep(){
    esp_deep_sleep(1000000LL * 60 * sleepMinutes);
}

void updateClock() {
   // Test Epd class with partial display
   display.init(true);
   display.setRotation(3);
   display.setFont(&Ubuntu_M20pt8b);
   display.setTextColor(EPD_BLACK);
   display.setCursor(80,50);
   display.print("30 Jun");
   // TODO Fix partial update for this epaper:
   //display.updateWindow(90, 80, 100, 30, true);
   

   // NVS to char array. Extract from NVS value and pad with 0 to string in case <10
   char hour[3];
   itoa(nvs_hour, hour, 10);

   char minute[3];
   itoa(nvs_minute, minute, 10);
   
   printf("%s:%s -> Sending to epaper\n", hour, minute);
   display.setCursor(90,80);
   display.print(hour);
   display.print(":");
   display.print(minute);
   display.update();
}

void app_main(void)
{
   printf("Demo sleeping clock\n");

       // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

        // Read stored
        printf("Reading minutes from NVS ... ");

        nvs_get_i8(my_handle, "h", &nvs_hour);
        err = nvs_get_i8(my_handle, "m", &nvs_minute);
        switch (err) {
            case ESP_OK:
                printf("NVS Time %d:%d\n", nvs_hour, nvs_minute);
                
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

      // After reading let's print the hour:
      updateClock();

        // Write
        printf("Updating restart counter in NVS ... ");
        nvs_minute+=sleepMinutes;
        // TODO Keep in mind that here sleepMinutes can be > 60 and that overpassing minutes need to be summed to 0
        if (nvs_minute>60) {
           nvs_hour++;
           nvs_minute = 0;
        }
        if (nvs_hour>23) {
           nvs_hour = 0;
        }
        err = nvs_set_i8(my_handle, "m", nvs_minute);
        printf((err != ESP_OK) ? "Failed saving minutes!\n" : "Done storing minutes\n");
         
        err = nvs_set_i8(my_handle, "h", nvs_hour);
        printf((err != ESP_OK) ? "Failed saving hour!\n" : "Done storing hour\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

   // TODO: Get initial time from internet 

   printf("deepsleep for %d minutes\n", sleepMinutes);
   deepsleep();
}

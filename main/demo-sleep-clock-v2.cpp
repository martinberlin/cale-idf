/*
 * - - - - - - - - Deepsleep clock example - - - - - v1 is intented for simple wristwatch  - 
 * Please note that the intention of this clock is not to be precise. 
 * It uses the ability of ESP32 to deepsleep combined with the epaper persistance
 * to make a simple clock that consumes as minimum as possible.
 * Just a simple: Sleep every N minutes, increment EPROM variable, refresh epaper.
 * And once a day or every hour, a single HTTP request to sync the hour online. 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include <sstream>
// Non-Volatile Storage (NVS) - borrrowed from esp-idf/examples/storage/nvs_rw_value
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
// - - - - HTTP Client
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_sleep.h"

bool debugVerbose = false;
// TinyPICO.com Dotstar or S2 with Neopixel led. Turn down power and set data /clk Gpios
#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12

// Important configuration. The class should match your epaper display model:
//#include <gdew075T7.h>
//#include "gdeh0213b73.h"
#include "gdeh0154d67.h"
EpdSpi io;
//Gdeh0213b73 display(io);
Gdeh0154d67 display(io);

// HTTP Request constants. Update Europe/Berlin with your timezone v
// Time: HHmm  -> 0800 (8 AM)   Time + Day 0800Fri 17, Jul
const char* timeQuery = "http://fs.fasani.de/api/?q=date&timezone=Europe/Berlin&f=HiD+d,+M";
// Represents the sizeof D+d,+M Ex: Sun 19, Jul  (11 chars + \0 null terminator)
char nvs_day_month[15];

// Clock will refresh each N minutes. Use one if you want a more realtime digital clock (But battery will last less)
int sleepMinutes = 2;

// At what time your CLOCK will get in Sync with the internet time?
// Clock syncs with internet time in this two SyncHours. Leave it on -1 to avoid internet Sync (Leave at least one set otherwise it will never get synchronized)
uint8_t syncHour1 = 0;        // IMPORTANT: Leave it on 0 for the first run!    On -1 to not sync at this hour
uint8_t syncHour2 = 6;         // Same here, 2nd request to Sync hour 
bool forceSync = false;        // IMPORTANT: Should be always false since on true is for debugging and will sync on every wakeup!
// This microsCorrection represents the program time and will be discounted from deepsleep
// Fine correction: Handle with care since this will be corrected on each sleepMinutes period
int64_t microsCorrection = -300000; // 0.3 predicted boot time

/*
 CLOCK Appearance - - - - - - - - - -
       
       9:02 -> textColor
*/
bool supportsPartialUpdate = false;      // If updateWindow does not work good for your display model turns this to false
uint16_t backgroundColor = EPD_WHITE;
uint16_t textColor = EPD_BLACK;
// Adafruit GFX Font selection - - - - - -
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h> // Day, Month
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>  // Last Sync message - Still not fully implemented
// Main digital clock hour font:
#include <Fonts/ubuntu/Ubuntu_M24pt8b.h> // HH:mm
#include <Fonts/ubuntu/Ubuntu_M36pt7b.h> // HH:mm
#include <Fonts/ubuntu/Ubuntu_M48pt8b.h> // HH:mm bigger in 48pt -> default selection
// HH:MM font size - Select between 24 and 48. It should match the previously defined fonts size
uint8_t fontSize = 36;

// HTTP_EVENT_ON_DATA callback needs to know what information is going to parse - UPDATE: Now parses always hour + date
// - - - - - - - - On 1: time  2: day, month
uint8_t onDataCheck = 1;
// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE  128
uint64_t startTime = 0;
uint16_t countDataEventCalls = 0;
static const char *TAG = "CALE CLOCK";
char espIpAddress[16];
bool espIsOnline = false;

// Values that will be stored in NVS - defaults should come initially from timequery (external HTTP request)
int8_t nvs_hour = 0;
int8_t nvs_minute = 0;

char nvs_last_sync_message[22];

// Flag to know that we've synced the hour with timeQuery request
int8_t nvs_last_sync_hour = 0;

size_t sizeof_last_sync_message = sizeof(nvs_last_sync_message);
size_t sizeof_day_month = sizeof(nvs_day_month);

extern "C"
{
   void app_main();
}

void deepsleep(){
    esp_deep_sleep(1000000LL * 60 * sleepMinutes - microsCorrection);
}

void updateClock() {
    // Half of display -NN should be the sum of pix per font
   uint8_t fontSpace = (fontSize/2); // Calculate aprox. how much space we need per font Character

   display.init(debugVerbose);
   display.fillScreen(backgroundColor);
   display.setRotation(CONFIG_DISPLAY_ROTATION); // Set this in "Cale configuration" -> idf.py menuconfig
   display.setFont(&Ubuntu_M16pt8b);
   display.setTextColor(textColor);

   // Day 01, Month  cursor location x,y
   switch(display.width()) {
       case 200:
        display.setCursor(10,52);
       break;
       case 250:
        display.setCursor(40,25);
       break;
       // Add more case's to adapt the cursor to your display size
       default: 
        display.setCursor(40,30);
       break;
   }
   
   
   if (debugVerbose) {
    printf("updateClock() called\n");
    printf("display.print() Day, month: %s\n\n", nvs_day_month);
    }
    
    display.print(nvs_day_month);

   /**
    * set font depending on selected fontSize
    */
   switch (fontSize)
   {
       /* Bigger font */
   case 48:
       display.setFont(&Ubuntu_M48pt8b);
       break;
   case 36:
       display.setFont(&Ubuntu_M36pt7b);
       break;
   case 24:
       display.setFont(&Ubuntu_M24pt8b);
       break;
   default:
       ESP_LOGE(TAG, "fontSize selection: %d is not defined. Please select 24 or 48 or define new fonts", fontSize);
       break;
   }
   // HH:mm cursor location depending on display width. Add more case's to adapt the cursor to your display size
   switch(display.width()) {
       case 200:
        display.setCursor(5, 120);
       break;
       case 250:
        display.setCursor(36, 86);
       break;
       default:
        display.setCursor(90, 80);
       break;
   }
   
   // NVS to char array. Extract from NVS value and pad with 0 to string in case <10
   char hour[3];
   char hourBuffer[3];
   // Convert the int into a char array
   itoa(nvs_hour, hour, 10);
   if (nvs_hour<10) {
      strlcpy(hourBuffer,    "0", sizeof(hourBuffer));
      strlcat(hourBuffer, hour, sizeof(hourBuffer));
   } else {
      strlcpy(hourBuffer, hour, sizeof(hourBuffer));
   }

   char minute[3];
   char minuteBuffer[3];
   itoa(nvs_minute, minute, 10);
   if (nvs_minute<10) {
      strlcpy(minuteBuffer,    "0", sizeof(minuteBuffer));
      strlcat(minuteBuffer, minute, sizeof(minuteBuffer));
   } else {
      strlcpy(minuteBuffer, minute, sizeof(minuteBuffer));
   } 
   
   if (debugVerbose) printf("%s:%s -> Sending to epaper\n", hourBuffer, minuteBuffer);

   display.print(hourBuffer);
   display.print(":");
   display.print(minuteBuffer);

   // Show last update message
   /* display.setFont(&Ubuntu_M8pt8b);
   display.setCursor(10,display.height()-20);
   display.print(nvs_last_sync_message); */

    // Draw rectangles for hour
   // Sizes are calculated dividing the screen in equal parts so it may not be perfect for all models
   //uint8_t rectWday  = display.width()/7; // 7 rectangles for each day of the week
   uint8_t rectWhour = (display.width()-2)/12; // 12 rectangles for each hour
   uint8_t rectXHour = 0;
   uint8_t rectYHour = display.height()-20;
   uint8_t rectHeightHour = 6;
   uint8_t hour12convert = 0;
   uint16_t widthHour = 0;

   for(uint8_t hourI = 1; hourI < 13; hourI++) {
       hour12convert = (nvs_hour>12) ? nvs_hour-12 : nvs_hour;
       rectXHour = (rectWhour*(hourI-1) == 0) ? 1 : rectWhour*(hourI-1);
    
     if (hourI == hour12convert) {
        widthHour = rectWhour*nvs_minute/59;
        display.fillRect(rectXHour, rectYHour, widthHour,rectHeightHour,textColor);
        printf("hourI: %d rectXHour: %d min: %d rectWhour: %d widthHour: %d\n", 
                    hourI, rectXHour, nvs_minute, rectWhour, widthHour);
     } 
     display.drawRect(rectXHour, rectYHour, rectWhour,rectHeightHour,textColor); 
   }
   
   // Partial update box calculation
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t w = display.width()-1;             // Update width should be smaller than full width() of the display
    uint16_t h = display.height()/2+fontSpace;
   // NOTE for other display models: 
   // If you use another display, bigger or smaller, than the demo you will need to change the updateWindow box coordinates
   switch (onDataCheck)
   {
       /* partial update    x  y  width               height                  */
   case 1:
       
       if (supportsPartialUpdate) {
           printf("HH:MM updateWindow(%d, %d, %d, %d)\n",x,y,w,h);
        display.updateWindow(x, y, w, h, true);
       } else {
        display.update(); 
       }
       //
       // Let's do a faster refresh using awesome updateWindow method (Thanks Jean-Marc for awesome example in GxEPD)
       // Note this x,y,width,height coordinates represent the bounding box where the update takes place:
       break;
       /* Day N, Month update full display (Partial leaves with the time a small background difference on the updateWindow) */
   case 2:
       printf("Day, Month changes: update() full epaper\n");
       display.update();
       break;
   }
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    char output_buffer[HTTP_RECEIVE_BUFFER_SIZE]; // Buffer to store HTTP response

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_DATA:
       ++countDataEventCalls;
         if (countDataEventCalls == 1)
        {
            startTime = esp_timer_get_time();
        }

        ESP_LOGI(TAG, "DATA CALLS: %d length:%d\n", countDataEventCalls, evt->data_len);
        memcpy(output_buffer, evt->data, evt->data_len);

        printf("\nInternet time Syncronization: ");
        for (uint8_t c=0; c<evt->data_len; c++){
           printf("%c", output_buffer[c]);
        }
        printf("\n");
        

        // Hour output_buffer[0] and output_buffer[1]. Both need null terminator in order for atoi to work correctly
        char hour[2];
        hour[0] = output_buffer[0];
        hour[1] = output_buffer[1];
        hour[2] = '\0';
        nvs_hour = atoi(hour);
        
        char min[2];
        min[0] = output_buffer[2];
        min[1] = output_buffer[3];
        min[2] = '\0';
        nvs_minute = atoi(min);
        
        for (uint8_t c=4; c < sizeof(nvs_day_month); c++){
            nvs_day_month[c-4] = output_buffer[c];
        }

        if (debugVerbose) printf("\nSYNC NVS time to hour: %d:%d\n\n", nvs_hour, nvs_minute);
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\nDownload took: %llu ms", (esp_timer_get_time()-startTime)/1000);
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
        break;
    }
    return ESP_OK;
}

/**
 * GET simple example, could be done also with POST, API accepts both
 */
static void http_get(const char * requestUrl)
{
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     */
    esp_http_client_config_t config = {
        .url = requestUrl,
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handler,
        .buffer_size = HTTP_RECEIVE_BUFFER_SIZE
        };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Perform the request. Will trigger the _http_event_handler event handler
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "\nREQUEST URL: %s\n\nHTTP GET Status = %d, content_length = %d\n",
                 requestUrl,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "\nHTTP GET request failed: %s", esp_err_to_name(err));
    }
}

/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(TAG, "Connect to the AP failed %d times. Going to deepsleep %d minutes", CONFIG_ESP_MAXIMUM_RETRY, CONFIG_DEEPSLEEP_MINUTES_AFTER_RENDER);
            deepsleep();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        sprintf(espIpAddress,  IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s\n", espIpAddress);
        espIsOnline = true;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    sprintf(reinterpret_cast<char *>(wifi_config.sta.ssid), CONFIG_ESP_WIFI_SSID);
    sprintf(reinterpret_cast<char *>(wifi_config.sta.password), CONFIG_ESP_WIFI_PASSWORD);
    wifi_config.sta.pmf_cfg.capable = true;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void app_main(void)
{
    uint64_t startTime = esp_timer_get_time();
    
    printf("ESP32 deepsleep clock\n");
    printf("Free heap memory: %d\n", xPortGetFreeHeapSize()); // Keep this above 100Kb to have a stable Firmware (Fonts take Heap!)

    // Turn off neopixel to keep consumption to the minimum
    gpio_set_direction((gpio_num_t)DOTSTAR_PWR, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode((gpio_num_t)DOTSTAR_CLK, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode((gpio_num_t)DOTSTAR_DATA, GPIO_PULLDOWN_ONLY);
    gpio_set_level((gpio_num_t)DOTSTAR_PWR, 0);

   // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        if (debugVerbose) printf("Done. Check if it's the hour to refresh from intenet times (%d or %d)\n", syncHour1, syncHour2);

        // Read stored
        nvs_get_i8(my_handle, "last_sync_h", &nvs_last_sync_hour);
        nvs_get_i8(my_handle, "h", &nvs_hour);
        nvs_get_str(my_handle, "last_sync_message", nvs_last_sync_message, &sizeof_last_sync_message);
        nvs_get_str(my_handle, "nvs_day_month", nvs_day_month, &sizeof_day_month);

        err = nvs_get_i8(my_handle, "m", &nvs_minute);
         // If the hour that comes from nvs matches one of the two syncHour's then syncronize with the www. Only if it was not already done!
         printf("LAST Sync hour: %d nvs_hour: %d nvs_minute: %d\n Last Sync message: %s\n\n", nvs_last_sync_hour, nvs_hour, nvs_minute, nvs_last_sync_message);

        // Sync on syncHour1 1 or 2 or when forceSync is true.
         if ((nvs_hour == syncHour1 || nvs_hour == syncHour2 || forceSync)  && (nvs_hour != nvs_last_sync_hour || forceSync)) {
            wifi_init_sta();
            uint8_t waitRounds = 0;
            while (espIsOnline==false && waitRounds<30) {
               vTaskDelay(100 / portTICK_PERIOD_MS);
               ++waitRounds;
            }
            
            http_get(timeQuery);
            // Mark a flag that the internet time was refreshed that is active for the rest of this hour
            nvs_set_i8(my_handle, "last_sync_h", nvs_hour);
            // Save message to store time of last update
            char lastSync[22];
            char hour[2];
            char min[2];
            itoa(nvs_hour, hour, 10);
            itoa(nvs_minute, min, 10);
            strlcpy(lastSync,  "Last sync at ", sizeof(lastSync));
            strlcat(lastSync,  hour, sizeof(lastSync));
            strlcat(lastSync,  " and ", sizeof(lastSync));
            strlcat(lastSync,  min, sizeof(lastSync));
            strlcat(lastSync,  " minutes.", sizeof(lastSync));

            nvs_set_str(my_handle, "last_sync_message", lastSync);

            printf("LAST SYNC: %s\n", lastSync);
         }


        switch (err) {
            case ESP_OK:
                if (debugVerbose) printf("NVS Time %d:%d\n", nvs_hour, nvs_minute);
                
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

      // After reading let's print the hour:
      updateClock();

        // Write NVS data so is read in next wakeup
        nvs_minute+=sleepMinutes;
        // TODO Keep in mind that here sleepMinutes can be > 60 and that overpassing minutes need to be summed to 0
        if (nvs_minute>59) {
           int8_t sumExtraMinutes = nvs_minute-60;
           nvs_hour++;
           nvs_minute = 0;
           // Reset flags since hour changed
           nvs_set_i8(my_handle, "last_sync_date", -1);
           nvs_set_i8(my_handle, "last_sync_h", -1);

           if (sumExtraMinutes>0) {
              nvs_minute+=sumExtraMinutes;
              printf("Summing %d minutes to new hour since last_minute+%d is equal to %d.\n", sumExtraMinutes, sleepMinutes, nvs_minute);
           }
        }
        // On 24 will be 00 hours
        if (nvs_hour>23) {
           nvs_hour = 0;
        }
        err = nvs_set_i8(my_handle, "m", nvs_minute);
        printf((err != ESP_OK) ? "Failed saving minutes!\n" : "Done storing minutes\n");
         
        err = nvs_set_i8(my_handle, "h", nvs_hour);
        printf((err != ESP_OK) ? "Failed saving hour!\n" : "Done storing hour\n");

        err = nvs_set_str(my_handle, "nvs_day_month", nvs_day_month);
        printf((err != ESP_OK) ? "Failed saving date!\n" : "Done storing date\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        ESP_LOGD(TAG, "Committing updates in NVS.");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

   // Calculate how much this program took to run and discount it from deepsleep
   uint32_t endTime = esp_timer_get_time();
   microsCorrection += endTime-startTime;

   printf("deepsleep for %d minutes. microsCorrection: %lld\n", sleepMinutes, microsCorrection);

   deepsleep();
}

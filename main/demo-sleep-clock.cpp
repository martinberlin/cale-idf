/*
 * - - - - - - - - Deepsleep clock example - - - - - - - - - - - - - - - - - - - - 
 * Please note that the intention of this clock is not to be precise. 
 * It uses the ability of ESP32 to deepsleep combined with the epaper persistance
 * to make a simple clock that consumes as minimum as possible.
 * Just a simple: Sleep every N minutes, increment EPROM variable, refresh epaper.
 * And once a day or every hour, a single HTTP request to sync the hour online. 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
// Epd display class
#include <gdew027w3.h>
#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

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

// HTTP Request constants
// Time query: HHmm  -> 0800 (8 AM)
const char* timeQuery = "http://fs.fasani.de/api/?q=date&timezone=Europe/Berlin&f=Hi";
// Day N, month - TODO for later maybe just combine it on one request
const char* dayQuery = "http://fs.fasani.de/api/?q=date&timezone=Europe/Berlin&f=l+d,%20F";

// Clock will refresh each N minutes
int sleepMinutes = 5;
// Clock will sync with internet time in this two Sync Hours. Leave it on 0 to avoid internet Sync (Leave at least one otherwise it will defer too much from actual time)
uint8_t syncHour1 = 9;     // At 9 in the morning the clock will Sync with internet time. Use it the time you most look at the clock if it's not perfect when you sleep...who cares?
uint8_t syncHour2 = 20;
uint8_t lastSyncHour = 0;  // Flag to know that we've synced the hour with timeQuery request

// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE 128
uint64_t startTime = 0;
uint16_t countDataEventCalls = 0;
static const char *TAG = "CALE CLOCK";
char espIpAddress[16];

EpdSpi io;
Gdew027w3 display(io);

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
   char hourBuffer[3];
   // Convert the int into a char array
   itoa(nvs_hour, hour, 10);
   if (nvs_hour<9) {
      strlcpy(hourBuffer,    "0", sizeof(hourBuffer));
      strlcat(hourBuffer, hour, sizeof(hourBuffer));
   } else {
   strlcpy(hourBuffer, hour, sizeof(hourBuffer));
   }

   char minute[3];
   char minuteBuffer[3];
   itoa(nvs_minute, minute, 10);
   if (nvs_minute<9) {
      strlcpy(minuteBuffer,    "0", sizeof(minuteBuffer));
      strlcat(minuteBuffer, minute, sizeof(minuteBuffer));
   } else {
   strlcpy(minuteBuffer, minute, sizeof(minuteBuffer));
   } 
   //clockLeadingZeros(nvs_minute, &minuteBuffer); // Incorrect way to do it (bad designed function)
   
   printf("%s:%s -> Sending to epaper\n", hourBuffer, minuteBuffer);
   display.setCursor(90,80);
   display.print(hourBuffer);
   display.print(":");
   display.print(minuteBuffer);
   display.update();
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
    // Copy the response into the buffer
        ESP_LOGI(TAG, "DATA CALLS: %d length:%d\n", countDataEventCalls, evt->data_len);
        memcpy(output_buffer, evt->data, evt->data_len);
        for (uint8_t c=0; c<evt->data_len; c++){
           printf("%c", output_buffer[c]);
        }
        printf("\nEND OF DATA - - - -\n");

        break;

       case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\nDownload took: %llu ms"
        , (esp_timer_get_time()-startTime)/1000);
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
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
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

    
   ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
   // Just a test, we need to start internet only to sync Time
   /* wifi_init_sta();
   http_get(timeQuery); */


    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done. Check if it's the hour to refresh from intenet times (%d or %d)\n", syncHour1, syncHour2);

        // Read stored
        printf("Reading minutes from NVS ... ");

        nvs_get_i8(my_handle, "h", &nvs_hour);
        err = nvs_get_i8(my_handle, "m", &nvs_minute);
         // If the hour that comes from nvs matches one of the two syncHour's then syncronize with the www
         if (nvs_hour == syncHour1 || nvs_hour == syncHour2) {
            wifi_init_sta();
            http_get(timeQuery);
            // TODO Mark a flag that the internet time was refreshed that is active for the rest of this hour
            lastSyncHour = nvs_hour;
         }

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
        if (nvs_minute>59) {
           uint8_t last_minutes = nvs_minute;
           int8_t sumExtraMinutes = nvs_minute-60;
           nvs_hour++;
           nvs_minute = 0;
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

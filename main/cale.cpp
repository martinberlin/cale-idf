#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
// - - - - HTTP Client
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_tls.h"
#include "esp_http_client.h"
//#include <wave12i48.h>
// Should match with your epaper module, size
//#include <gdew075T7.h>
#include <gdew027w3.h>

// Multi-SPI 4 channels EPD only
//Epd4Spi io;
//Wave12I48 display(io);
// Single SPI EPD
EpdSpi io;
//Gdew075T7 display(io);
Gdew027w3 display(io);
#include <Fonts/FreeMonoBold24pt7b.h>

extern "C" {
   void app_main();
}

static const char *TAG = "CALE";

#define MAX_HTTP_OUTPUT_BUFFER 1024

uint16_t countDataEventCalls=0;
uint32_t countDataBytes=0;
bool willParse = true;
struct BmpHeader
{
   uint32_t fileSize;
   uint32_t imageOffset;
   uint32_t headerSize;
   uint32_t width;
   uint32_t height;
   uint16_t planes;
   uint16_t depth;
   uint32_t format;
} bmp;

uint16_t read16(uint8_t output_buffer[512], uint8_t startPointer)
{
  // BMP data is stored little-endian
  uint16_t result;
  ((uint8_t *)&result)[0] = output_buffer[startPointer];   // LSB
  ((uint8_t *)&result)[1] = output_buffer[startPointer+1]; // MSB
  return result;
}

uint32_t read32(uint8_t output_buffer[512], uint8_t startPointer)
{
    printf("read32: %x %x %x %x\n", output_buffer[startPointer],
    output_buffer[startPointer+1],output_buffer[startPointer+2],output_buffer[startPointer+3]);
    
  uint32_t result;
  ((uint8_t *)&result)[0] = output_buffer[startPointer]; // LSB
  ((uint8_t *)&result)[1] = output_buffer[startPointer+1];
  ((uint8_t *)&result)[2] = output_buffer[startPointer+2];
  ((uint8_t *)&result)[3] = output_buffer[startPointer+3]; // MSB
  return result;
}

// BMP reading flags
bool valid = false; // valid format to be handled
bool flip = true;   // bitmap is stored bottom-to-top
bool with_color = false; // Candidate for Kconfig
uint32_t rowSize;
uint32_t rowByteCounter;
uint16_t w;
uint16_t h;
uint8_t bitmask = 0xFF;
uint8_t bitshift;
uint16_t red, green, blue;
bool whitish, colored;
//uint32_t rowPosition;
uint16_t drawX = 0;
uint16_t drawY = 0;
// bPointer is 34 only first time when the headers come. Then is 0!
uint16_t bPointer = 34; // Byte pointer - Attention drawPixel has uint16_t
uint16_t imageBytesRead = 0;

uint32_t in_bytes = 0;
uint8_t in_byte = 0; // for depth <= 8
uint8_t in_bits = 0; // for depth <= 8

static const uint16_t input_buffer_pixels = 640; // may affect performance
static const uint16_t max_palette_pixels = 256; // for depth <= 8
uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint16_t totalDrawPixels = 0;
int color = EPD_WHITE;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    uint8_t output_buffer[512];  // Buffer to store response 

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ++countDataEventCalls;
            ESP_LOGI(TAG, "%d len:%d\n", countDataEventCalls, evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            //if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
               memcpy(output_buffer, evt->data, evt->data_len);
               if (countDataEventCalls==1) {
                    // display.fillScreen(EPD_WHITE);
                    // Read BMP header -In total 34 bytes header
                    bmp.fileSize    = read32(output_buffer,2);
                    bmp.imageOffset = read32(output_buffer,10);
                    bmp.headerSize  = read32(output_buffer,14);
                    bmp.width       = read32(output_buffer,18);
                    bmp.height      = read32(output_buffer,22);
                    bmp.planes      = read16(output_buffer,26);
                    bmp.depth       = read16(output_buffer,28);
                    bmp.format      = read32(output_buffer,30);

                    drawY = bmp.height;
                    ESP_LOGI(TAG, "BMP HEADERS\nfilesize:%d\noffset:%d\nW:%d\nH:%d\nplanes:%d\ndepth:%d\nformat:%d\n", 
                    bmp.fileSize,bmp.imageOffset,bmp.width,bmp.height,bmp.planes,bmp.depth,bmp.format);

                    if (((bmp.planes == 1) && ((bmp.format == 0) || (bmp.format == 3))) == false) { // uncompressed is handled, 565 also, rest no.
                      willParse = false;
                      ESP_LOGE(TAG,"BMP NOT SUPPORTED (compressed formats not handled)\n");
                    }

                rowSize = (bmp.width * bmp.depth / 8 + 3) & ~3;
                if (bmp.depth < 8) rowSize = ((bmp.width * bmp.depth + 8 - bmp.depth) / 8 + 3) & ~3;

                printf("ROW Size %d\n", rowSize);

                if (bmp.height < 0)
                {
                    bmp.height = -bmp.height;
                    flip = false;
                }
                w = bmp.width;
                h = bmp.height;
                if ((w - 1) >= display.width())  w = display.width();
                if ((h - 1) >= display.height()) h = display.height();
                
                bitshift = 8 - bmp.depth;
                
                if (bmp.depth == 1) with_color = false;
               
                if (bmp.depth <= 8){
                if (bmp.depth < 8) bitmask >>= bmp.depth;
                // Color-palette location:
                bPointer = bmp.imageOffset - (4 << bmp.depth);
                printf("Palette location: %d\n\n",bPointer);

                for (uint16_t pn = 0; pn < (1 << bmp.depth); pn++){
                    blue  = output_buffer[bPointer++];
                    green = output_buffer[bPointer++];
                    red   = output_buffer[bPointer++];
                    bPointer++;

                    whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                    colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                    if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
                    mono_palette_buffer[pn / 8] |= whitish << pn % 8;
                    if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
                    color_palette_buffer[pn / 8] |= colored << pn % 8;
                    // DEBUG Colors
                    //printf("Colors palette:\n"); printf("RED: %x\n",red); printf("GREEN: %x\n",green); printf("BLUE: %x\n",blue);
                    //printf("whitish: %x colored: %x\n\n", whitish,colored);
                }
                }

                //rowPosition = flip ? bmp.imageOffset + (bmp.height - h) * rowSize : bmp.imageOffset;

                // Important we need to start reading the image where the header offset marks
                bPointer = bmp.imageOffset;
               } else {
                   bPointer = 0;
               }
               if (!willParse) return ESP_FAIL;

                printf("\n--> bPointer %d\n_inX: %d _inY: %d\n", bPointer, drawX, drawY);

            // LOOP all the received Buffer but start on ImageOffset if first call

            for (uint32_t byteIndex=bPointer; byteIndex <= evt->data_len; ++byteIndex) {
                in_byte = output_buffer[byteIndex];
                if (byteIndex==0) {
                printf("1ST BYTE: %x\n", in_byte);
                }

                in_bits = 8;

                switch (bmp.depth){
                    case 1:
                    case 4:
                    case 8:
                    {
                        while (in_bits!=0) {
                         
                            uint16_t pn = (in_byte >> bitshift) & bitmask;
                            whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                            colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                            in_byte <<= bmp.depth;
                            in_bits -= bmp.depth;

                            if (whitish){
                                color = EPD_WHITE;
                            } else if (colored && with_color){
                                color = EPD_RED;
                            }
                            else {
                                color = EPD_BLACK;
                            }
                            
                            
                            display.drawPixel(drawX, drawY, color);

                            /* if (drawX==0) {
                                // Draw a fake black point at 0,Y
                                display.drawPixel(drawX, drawY, EPD_BLACK);
                            } */


                            totalDrawPixels++;
                            ++drawX;

                            // bmp.width reached? Then go one line up
                            if (drawX > bmp.width) {
                                printf("dX:%d Y:%d\n", drawX, drawY);
                                drawX = 0;
                                rowByteCounter = 0;
                                --drawY;
                            } 

                        }

                    }
                    break;
                }
                
                rowByteCounter++;
                imageBytesRead++;
                
            }

            printf("Total drawPixel calls: %d\noutX: %d outY: %d\n", totalDrawPixels, drawX, drawY);

            // Hexa dump
            //ESP_LOG_BUFFER_HEX(TAG, output_buffer, evt->data_len);
            // On 4 bits depth image returns chunked response
            /* } else {
                ESP_LOGI(TAG,"Is chunked response\n");
            } */

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\n");
            display.update();
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
           
            break;
    }
    return ESP_OK;
}


static void http_post(void)
{
    // w content len:  http://img.cale.es/jpg/fasani/5ea1dec401890
    // BMP :           http://img.cale.es/bmp/fasani/5e5926e25a985
  
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     * 
      .host = "httpbin.org",
       .path = "/get",
       2.13 http://img.cale.es/bmp/fasani/5e793990af85d -> 1st test
     */
    esp_http_client_config_t config = {
        .url = "http://cale.es/img/test/1.bmp",
        .event_handler = _http_event_handler
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "\nHTTP GET Status = %d, content_length = %d\n",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "\nHTTP GET request failed: %s", esp_err_to_name(err));
    }
    //ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
}



/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
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
    sprintf (reinterpret_cast<char*>(wifi_config.sta.ssid), CONFIG_ESP_WIFI_SSID );
    sprintf (reinterpret_cast<char*>(wifi_config.sta.password), CONFIG_ESP_WIFI_PASSWORD);
    wifi_config.sta.pmf_cfg.capable = true;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    printf("Free heap: %d\n",xPortGetFreeHeapSize());
    display.init(true);
    display.setRotation(3);

    http_post();

   // Just test if Epd works:
   /* display.setRotation(2);
   display.setCursor(10,25);
   display.setTextColor(EPD_BLACK);
   display.setFont(&FreeMonoBold24pt7b);
   display.println("CalEPD");
   display.update();  */
}

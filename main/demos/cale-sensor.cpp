/**
 * This is just a variation of the official CALE Firmware in main/
 * Explores how you can add a sensor, in this example a PIR Motion Detection Sensor AM312
 * That is attached to an interrupt and triggers a system restart
 * The DEMO goal is to be used with https://cale.es/apis > Image gallery so it can make a new request based on a user interaction
 * 
 * Disclaimer: The ideal would be to call http_post() again so it saves time and does not need to connect to WiFi again but
 * if failed most probably since it's called from an interrupt (And you cannot even do a printf on an event alike)
 */
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
#include "esp_sleep.h"
#include "soc/rtc_wdt.h"       // Watchdog control
// - - - - HTTP Client includes:
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_tls.h"
#include "esp_http_client.h"
/**
 * Should match your display model. Check repository WiKi: https://github.com/martinberlin/cale-idf/wiki
 * Needs 3 things: 
 * 1. Include the right class (Check Wiki for supported models)
 * 2. Instantiate io class, below an example for Good Display/Waveshare epapers
 * 3. Instantiate the epaper class itself. After this you can call display.METHOD from any part of your program
 */
// 1 channel SPI epaper displays example:
#include <gdew075c64.h>
//#include <gdew042t2.h>
//#include <gdew027w3.h>
//#include "gdeh0213b73.h"
EpdSpi io;
//Gdew042t2 display(io);
Gdew075C64 display(io);
// Plastic Logic test: Check cale-grayscale.cpp

// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
//#include "wave12i48.h" // Only to use with Edp4Spi IO
//Epd4Spi io;
//Wave12I48 display(io);

// BMP debug Mode: Turn false for production since it will make things slower and dump Serial debug
bool bmpDebug = false;

// Please notice that enabling this will stay on WiFi to make next request as soon as possible
bool enable_sensor_1 = true;    // Activate only if you have a rising sensor in pin defined at ESP_TRIGGER_SENSOR1

bool activate_sensor_1 = false; // Handler flag to avoid activating interrupt when is not ready
bool trigger_sensor_1 = false;  // Handler on true will call again http_post() for a new image
// IP is sent per post for logging purpouses. Authentication: Bearer token in the headers
char espIpAddress[16];
char bearerToken[74] = "";
// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE 1024

extern "C"
{
    void app_main();
}

static const char *TAG = "CALE";

uint16_t countDataEventCalls = 0;

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
    ((uint8_t *)&result)[0] = output_buffer[startPointer];     // LSB
    ((uint8_t *)&result)[1] = output_buffer[startPointer + 1]; // MSB
    return result;
}

uint32_t read32(uint8_t output_buffer[512], uint8_t startPointer)
{
    //Debug - Leave disabled to avoid Serial output
    //printf("read32: %x %x %x %x\n", output_buffer[startPointer],output_buffer[startPointer+1],output_buffer[startPointer+2],output_buffer[startPointer+3]);
    uint32_t result;
    ((uint8_t *)&result)[0] = output_buffer[startPointer]; // LSB
    ((uint8_t *)&result)[1] = output_buffer[startPointer + 1];
    ((uint8_t *)&result)[2] = output_buffer[startPointer + 2];
    ((uint8_t *)&result)[3] = output_buffer[startPointer + 3]; // MSB
    return result;
}

bool with_color = true; // Candidate for Kconfig
uint32_t rowSize;
uint32_t rowByteCounter=0;
uint16_t w, h;
uint8_t bitmask = 0xFF;
uint8_t bitshift;
uint16_t red, green, blue;
bool whitish, colored;
uint16_t drawX = 0;
uint16_t drawY = 0;
uint16_t bPointer = 0; // Byte pointer - Attention drawPixel has uint16_t
uint16_t imageBytesRead = 0;
uint32_t dataLenTotal = 0;
uint32_t in_bytes = 0;
uint8_t in_byte = 0; // for depth <= 8
uint8_t in_bits = 0; // for depth <= 8
bool isReadingImage = false;
bool isSupportedBitmap = true;
bool isPaddingAware = false;

static const uint16_t input_buffer_pixels = 640;      // may affect performance
static const uint16_t max_palette_pixels = 256;       // for depth <= 8
uint8_t mono_palette_buffer[max_palette_pixels / 8];  // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint32_t totalDrawPixels = 0;
int color = EPD_WHITE;
uint64_t startTime = 0;

void deepsleep(){
    esp_deep_sleep(1000000LL * 60 * CONFIG_DEEPSLEEP_MINUTES_AFTER_RENDER);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    uint8_t output_buffer[HTTP_RECEIVE_BUFFER_SIZE]; // Buffer to store HTTP response

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
        if (countDataEventCalls%10==0) {
        ESP_LOGI(TAG, "%d len:%d\n", countDataEventCalls, evt->data_len); }
        dataLenTotal += evt->data_len;
        // Unless bmp.imageOffset initial skip we start reading stream always on byte pointer 0:
        bPointer = 0;
        // Copy the response into the buffer
        memcpy(output_buffer, evt->data, evt->data_len);

        if (countDataEventCalls == 1)
        {
            startTime = esp_timer_get_time();
            // Read BMP header -In total 34 bytes header
            bmp.fileSize = read32(output_buffer, 2);
            bmp.imageOffset = read32(output_buffer, 10);
            bmp.headerSize = read32(output_buffer, 14);
            bmp.width = read32(output_buffer, 18);
            bmp.height = read32(output_buffer, 22);
            bmp.planes = read16(output_buffer, 26);
            bmp.depth = read16(output_buffer, 28);
            bmp.format = read32(output_buffer, 30);

            drawY = bmp.height;
            ESP_LOGI(TAG, "BMP HEADERS\nfilesize:%d\noffset:%d\nW:%d\nH:%d\nplanes:%d\ndepth:%d\nformat:%d\n",
                     bmp.fileSize, bmp.imageOffset, bmp.width, bmp.height, bmp.planes, bmp.depth, bmp.format);

            if (bmp.depth == 1)
            {
                isPaddingAware = true;
                ESP_LOGI(TAG, "BMP isPaddingAware:  1 bit depth are 4 bit padded");
            }
            if (((bmp.planes == 1) && ((bmp.format == 0) || (bmp.format == 3))) == false)
            { // Only uncompressed is handled
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP NOT SUPPORTED: Compressed formats not handled.\nBMP NOT SUPPORTED: Only planes==1, format 0 or 3\n");
            }
            if (bmp.depth > 8)
            {
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP DEPTH %d: Only 1, 4, and 8 bits depth are supported.\n", bmp.depth);
            }

            rowSize = (bmp.width * bmp.depth / 8 + 3) & ~3;
            if (bmp.depth < 8)
                rowSize = ((bmp.width * bmp.depth + 8 - bmp.depth) / 8 + 3) & ~3;

            if (bmpDebug)
                ets_printf("ROW Size %d\n", rowSize);

            if (bmp.height < 0)
            {
                bmp.height = -bmp.height;
            }
            w = bmp.width;
            h = bmp.height;
            if ((w - 1) >= display.width())
                w = display.width();
            if ((h - 1) >= display.height())
                h = display.height();

            bitshift = 8 - bmp.depth;

            if (bmp.depth == 1)
                with_color = false;

            if (bmp.depth <= 8)
            {
                if (bmp.depth < 8)
                    bitmask >>= bmp.depth;
                // Color-palette location:
                bPointer = bmp.imageOffset - (4 << bmp.depth);
                if (bmpDebug)
                    ets_printf("Palette location: %d\n\n", bPointer);

                for (uint16_t pn = 0; pn < (1 << bmp.depth); pn++)
                {
                    blue = output_buffer[bPointer++];
                    green = output_buffer[bPointer++];
                    red = output_buffer[bPointer++];
                    bPointer++;

                    whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                    colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
                    if (0 == pn % 8) {
                        mono_palette_buffer[pn / 8] = 0;
                        color_palette_buffer[pn / 8] = 0;
                    }
                        
                    mono_palette_buffer[pn / 8]  |= whitish << pn % 8;                       
                    color_palette_buffer[pn / 8] |= colored << pn % 8;

                    // DEBUG Colors - TODO: Double check Palette!!
                    if (bmpDebug)
                        ets_printf("0x00%x%x%x : %x, %x\n", red, green, blue, whitish, colored);
                }
            }
            imageBytesRead += evt->data_len;
        }
        if (!isSupportedBitmap)
            return ESP_FAIL;

        if (bmpDebug)
        {
            ets_printf("\n--> bPointer %d\n_inX: %d _inY: %d DATALEN TOTAL:%d bytesRead so far:%d\n",
                   bPointer, drawX, drawY, dataLenTotal, imageBytesRead);
            ets_printf("Is reading:%d\n", isReadingImage);
        }

        // Didn't arrived to imageOffset YET, it will in next calls of HTTP_EVENT_ON_DATA:
        if (dataLenTotal < bmp.imageOffset)
        {
            imageBytesRead = dataLenTotal;
            if (bmpDebug)
                ets_printf("IF read<offset UPDATE bytesRead:%d\n", imageBytesRead);
            return ESP_OK;
        }
        else
        {
            // Only move pointer once to set right offset
            if (countDataEventCalls == 1 && bmp.imageOffset < evt->data_len)
            {
                bPointer = bmp.imageOffset;
                isReadingImage = true;
                ets_printf("Offset comes in first DATA callback. bPointer: %d == bmp.imageOffset\n", bPointer);
            }
            if (!isReadingImage)
            {
                bPointer = bmp.imageOffset - imageBytesRead;
                imageBytesRead += bPointer;
                isReadingImage = true;
                ets_printf("Start reading image. bPointer: %d\n", bPointer);
            }
        }

        // LOOP all the received Buffer but start on ImageOffset if first call
        for (uint32_t byteIndex = bPointer; byteIndex < evt->data_len; ++byteIndex)
        {
            in_byte = output_buffer[byteIndex];
            // Dump only the first calls
            if (countDataEventCalls < 2 && bmpDebug)
            {
                ets_printf("L%d: BrsF:%d %x\n", byteIndex, imageBytesRead, in_byte);
            }
            in_bits = 8;

            switch (bmp.depth)
            {
            case 1:
            case 4:
            case 8:
            {
                while (in_bits != 0)
                {
                    uint16_t pn = (in_byte >> bitshift) & bitmask;
                    whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    in_byte <<= bmp.depth;
                    in_bits -= bmp.depth;

                    if (whitish)
                    {
                        color = EPD_WHITE;
                    }
                    else if (colored && with_color)
                    {
                        color = EPD_RED;
                    }
                    else
                    {
                        color = EPD_BLACK;
                    }

                    // bmp.width reached? Then go one line up (Is readed from bottom to top)
                    if (isPaddingAware)
                    { // 1 bit images are 4-bit padded (Filled usually with 0's)
                        if (drawX + 1 > rowSize * 8)
                        {
                            drawX = 0;
                            rowByteCounter = 0;
                            --drawY;
                        }
                    }
                    else
                    {
                        if (drawX + 1 > bmp.width)
                        {
                            drawX = 0;
                            rowByteCounter = 0;
                            --drawY;
                        }
                    }
                    // The ultimate mission: Send the X / Y pixel to the GFX Buffer
                    display.drawPixel(drawX, drawY, color);

                    // Extreme debug ;)
                    /* if (drawY>460 && drawX>= 700) {
                      ets_printf("%x ",color);
                    } */

                    totalDrawPixels++;
                    ++drawX;
                }
            }
            break;
            }

            rowByteCounter++;
            imageBytesRead++;
        }

        if (bmpDebug)
            ets_printf("Total drawPixel calls: %d\noutX: %d outY: %d\n", totalDrawPixels, drawX, drawY);

        // Hexa dump:
        //ESP_LOG_BUFFER_HEX(TAG, output_buffer, evt->data_len);
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\nDownload took: %llu ms\nRefresh and go to sleep %d minutes\n", (esp_timer_get_time()-startTime)/1000, CONFIG_DEEPSLEEP_MINUTES_AFTER_RENDER);
        display.update();

        if (bmpDebug) 
            ets_printf("Free heap after display render: %d\n", xPortGetFreeHeapSize());
        // Go to deepsleep after rendering
        vTaskDelay(15000 / portTICK_PERIOD_MS);
        
        if (CONFIG_ESP_TRIGGER_SENSOR1 && enable_sensor_1) {
            ets_printf("Activating sensor in GPIO %d\n", CONFIG_ESP_TRIGGER_SENSOR1);
            activate_sensor_1 = true;
        } else {
            deepsleep();
        }
        
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
        break;
    }
    return ESP_OK;
}

uint16_t no_cache = 0;

void reset_http_image_globals() {
    countDataEventCalls=0;
    dataLenTotal=0;
    imageBytesRead=0;
    isSupportedBitmap = true;
    isReadingImage = false;
    isPaddingAware = false;
    drawX = 0;
    drawY = 0;
    rowByteCounter=0;
    totalDrawPixels=0;
    bitmask = 0xFF;
    in_bytes = 0;
    in_byte = 0;
    in_bits = 0;
    printf("reset_http_image_globals called. Free heap: %d bytes\n",
    xPortGetFreeHeapSize());
}

static void http_post()
{
    activate_sensor_1 = false;
    trigger_sensor_1 = false;
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered. TESTs:
       http://img.cale.es/bmp/fasani/5e8cc4cf03d81  -> 4 bit 2.7 tests
       http://cale.es/img/test/1.bmp                -> vertical line
       http://cale.es/img/test/circle.bmp           -> Circle test
       timeout_ms set to 9 seconds since for large displays a dynamic BMP can take some seconds to be generated
     */
    
    // POST Send the IP for logging purpouses
    char post_data[22];
    uint8_t postsize = sizeof(post_data);
    strlcpy(post_data, "ip=", postsize);
    strlcat(post_data, espIpAddress, postsize);

    char post_url[200];
    strlcpy(post_url, CONFIG_CALE_SCREEN_URL, sizeof(post_url));
    // Is not a cache issue but for a static no-dynamic image this can come handy:
    strlcat(post_url, "?", sizeof(post_url));
    strlcat(post_url, std::to_string(no_cache).c_str(), sizeof(post_url));
    no_cache++;

    esp_http_client_config_t config = {
        .url = post_url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 9000,
        .event_handler = _http_event_handler,
        .buffer_size = HTTP_RECEIVE_BUFFER_SIZE
        };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Authentication: Bearer    
    strlcpy(bearerToken, "Bearer: ", sizeof(bearerToken));
    strlcat(bearerToken, CONFIG_CALE_BEARER_TOKEN, sizeof(bearerToken));
    
    printf("POST data: %s\n%s\nURL: %s\n\n", post_data, bearerToken, post_url);

    esp_http_client_set_header(client, "Authorization", bearerToken);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    // Here it hangs if you call it again from an interrupt:
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "\nIMAGE URL: %s\n\nHTTP GET Status = %d, content_length = %d\n",
                 CONFIG_CALE_SCREEN_URL,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "\nHTTP GET request failed: %s", esp_err_to_name(err));
    }
    // RESET global variables used in esp_http_client_init
    reset_http_image_globals();
    esp_http_client_cleanup(client);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // Request new image
    if (activate_sensor_1) {
        ets_printf("Interrupt sensor 1 called\n");
        trigger_sensor_1 = true;
        //http_post(); // Does not accept esp_http_client_perform from this interrupt. Calling it in a while(true) on app_main()
    } else {
        ets_printf("Sensor 1 not ready\n");
    }
}

/* FreeRTOS event group to signal when we are connected*/
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
    printf("CalEPD version: %s\n", CALEPD_VERSION);

    // Sensor that goes HIGH when detects something (Ex. triggering a new image request in an exposition)
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL<<CONFIG_ESP_TRIGGER_SENSOR1;  
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)1;
    gpio_config(&io_conf);
    
    // Install gpio isr service only if there is a sensor
    if (enable_sensor_1) {
        esp_err_t isr = gpio_install_isr_service(0);
        printf("ISR trigger install response: 0x%x\n", isr);
        gpio_isr_handler_add((gpio_num_t)CONFIG_ESP_TRIGGER_SENSOR1, gpio_isr_handler, (void*) 1);
    }

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    
    //  On  init(true) activates debug (And makes SPI communication slower too)
    display.init();
    display.setRotation(CONFIG_DISPLAY_ROTATION);
    // Show available Dynamic Random Access Memory available after display.init() - Both report same number
    printf("Free heap RAM: %d (After epaper instantiation)\n", xPortGetFreeHeapSize());

    http_post();

    if (enable_sensor_1) {
        while (activate_sensor_1)
        {
            if (trigger_sensor_1) {
                printf("New HTTP Post image request\n");
                http_post();
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            rtc_wdt_feed();
        }
    }
    
}

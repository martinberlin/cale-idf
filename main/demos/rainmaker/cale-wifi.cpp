/**
 * Cale version that uses Espressif RainMaker for the WiFi provisioning
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
#include "nvs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_sleep.h"
// - - - - HTTP Client includes:
#include "esp_err.h"
#include "esp_tls.h"
#include "esp_http_client.h"
// Rainmaker
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_ota.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_utils.h>
#include <app_wifi.h>
#include <esp_rmaker_factory.h>
#include <esp_rmaker_common_events.h>

#define DEVICE_PARAM_1 "Minutes till next refresh"
#define DEVICE_PARAM_WIFI_RESET "Turn slider to 100 to reset WiFi"
bool readyToRequestImage = false;
/**
 * Should match your display model. Check repository WiKi: https://github.com/martinberlin/cale-idf/wiki
 * Needs 3 things: 
 * 1. Include the right class (Check Wiki for supported models)
 * 2. Instantiate io class, below an example for Good Display/Waveshare epapers
 * 3. Instantiate the epaper class itself. After this you can call display.METHOD from any part of your program
 */
// 1 channel SPI epaper displays example:
//#include <gdew075T7.h>
#include <gdeh042Z96.h>
// Font always after display
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>

EpdSpi io;
Gdeh042Z96 display(io);

esp_rmaker_device_t *epaper_device;

// BMP debug Mode: Turn false for production since it will make things slower and dump Serial debug
const bool bmpDebug = false;

// Authentication: Bearer token in the headers
char bearerToken[74] = "";

// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE 1024

extern "C"
{
    void app_main();
}

static const char *TAG = "CALE";
// Values that will be stored in NVS - defaults here
nvs_handle_t nvs_h;
uint16_t nvs_minutes_till_refresh = 60;

uint16_t countDataEventCalls = 0;
uint32_t countDataBytes = 0;

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

// OTA Disabled
//extern const char ota_server_cert[] asm("_binary_server_crt_start");

void deepsleep(){
    nvs_get_u16(nvs_h, "mtr", &nvs_minutes_till_refresh);
    nvs_close(nvs_h);
    printf("Going to deepsleep %d minutes\n\n", nvs_minutes_till_refresh);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_deep_sleep(1000000LL * 60 * nvs_minutes_till_refresh);
}

/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        if (val.val.b == false) {
            deepsleep();
        }

    } else if (strcmp(param_name, DEVICE_PARAM_1) == 0) {
        ESP_LOGI(TAG, "%d for %s-%s",
                val.val.i, device_name, param_name);

        nvs_handle_t my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } 
        nvs_set_u16(my_handle, "mtr", (uint16_t) val.val.i);

        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "NVS Failed to store %d\n" : "NVS Stored %d\n", val.val.i);
        nvs_close(my_handle);    

    } else if (strcmp(param_name, DEVICE_PARAM_WIFI_RESET) == 0) {
        ESP_LOGI(TAG, "%d for %s-%s",
                val.val.i, device_name, param_name);
        if (val.val.i == 100) {
            printf("Reseting WiFi credentials. Please reprovision your device\n\n");
            esp_rmaker_wifi_reset(1,10);
        }

    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}


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
uint32_t rowByteCounter;
uint16_t w;
uint16_t h;
uint8_t bitmask = 0xFF;
uint8_t bitshift;
uint16_t red, green, blue;
bool whitish, colored;
uint16_t drawX = 0;
uint16_t drawY = 0;
uint16_t bPointer = 34; // Byte pointer - Attention drawPixel has uint16_t
uint16_t imageBytesRead = 0;
uint32_t dataLenTotal = 0;
uint32_t in_bytes = 0;
uint8_t in_byte = 0; // for depth <= 8
uint8_t in_bits = 0; // for depth <= 8
bool isReadingImage = false;
bool isSupportedBitmap = true;
bool isPaddingAware = false;
uint16_t forCount = 0;

static const uint16_t input_buffer_pixels = 640;      // may affect performance
static const uint16_t max_palette_pixels = 256;       // for depth <= 8
uint8_t mono_palette_buffer[max_palette_pixels / 8];  // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint16_t totalDrawPixels = 0;
int color = EPD_WHITE;
uint64_t startTime = 0;

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
                ESP_LOGI(TAG, "BMP isPaddingAware:  1 bit depth are 4 bit padded. Wikipedia gave me a lesson.");
            }
            if (((bmp.planes == 1) && ((bmp.format == 0) || (bmp.format == 3))) == false)
            { // uncompressed is handled
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP NOT SUPPORTED: Compressed formats not handled.\nBMP NOT SUPPORTED: Only planes==1, format 0 or 3\n");
            }
            if (bmp.depth > 8)
            {
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP DEPTH %d: Only 1, 4, and 8 bits depth are supported.\n", bmp.depth);
            }

            rowSize = ((bmp.width * bmp.depth + 8 - bmp.depth) / 8 + 3) & ~3;

            if (bmpDebug)
                printf("ROW Size %d\n", rowSize);

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
                    printf("Palette location: %d\n\n", bPointer);

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
                        printf("0x00%x%x%x : %x, %x\n", red, green, blue, whitish, colored);
                }
            }
            imageBytesRead += evt->data_len;
        }
        if (!isSupportedBitmap)
            return ESP_FAIL;

        if (bmpDebug)
        {
            printf("\n--> bPointer %d\n_inX: %d _inY: %d DATALEN TOTAL:%d bytesRead so far:%d\n",
                   bPointer, drawX, drawY, dataLenTotal, imageBytesRead);
            printf("Is reading image: %d\n", isReadingImage);
        }

        // Didn't arrived to imageOffset YET, it will in next calls of HTTP_EVENT_ON_DATA:
        if (dataLenTotal < bmp.imageOffset)
        {
            imageBytesRead = dataLenTotal;
            if (bmpDebug)
                printf("IF read<offset UPDATE bytesRead:%d\n", imageBytesRead);
            return ESP_OK;
        }
        else
        {
            // Only move pointer once to set right offset
            if (countDataEventCalls == 1 && bmp.imageOffset < evt->data_len)
            {
                bPointer = bmp.imageOffset;
                isReadingImage = true;
                printf("Offset comes in first DATA callback. bPointer: %d == bmp.imageOffset\n", bPointer);
            }
            if (!isReadingImage)
            {
                bPointer = bmp.imageOffset - imageBytesRead;
                imageBytesRead += bPointer;
                isReadingImage = true;
                printf("Start reading image. bPointer: %d\n", bPointer);
            }
        }
        forCount = 0;
        // LOOP all the received Buffer but start on ImageOffset if first call
        for (uint32_t byteIndex = bPointer; byteIndex < evt->data_len; ++byteIndex)
        {
            in_byte = output_buffer[byteIndex];
            // Dump only the first calls
            if (countDataEventCalls < 2 && bmpDebug)
            {
                printf("L%d: BrsF:%d %x\n", byteIndex, imageBytesRead, in_byte);
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
                        // if (drawX + 1 > bmp.width) -> Updated but not tested with 12.48 inch epaper
                        if (drawX + 1 > rowSize *2)
                        {
                            drawX = 0;
                            rowByteCounter = 0;
                            --drawY;
                        }
                    }
                    // The ultimate mission: Send the X / Y pixel to the GFX Buffer
                    display.drawPixel(drawX, drawY, color);
                    if (drawY == 0) break;

                    totalDrawPixels++;
                    ++drawX;
                }
            }
            break;
            }

            rowByteCounter++;
            imageBytesRead++;
            forCount++;
        }

        if (bmpDebug)
            printf("Total drawPixel calls: %d\noutX: %d outY: %d\n", totalDrawPixels, drawX, drawY);

        // Hexa dump:
        //ESP_LOG_BUFFER_HEX(TAG, output_buffer, evt->data_len);
        break;

    case HTTP_EVENT_ON_FINISH:
        countDataEventCalls=0;
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\nDownload took: %llu ms\nRefresh and go to sleep %d minutes\n", (esp_timer_get_time()-startTime)/1000, CONFIG_DEEPSLEEP_MINUTES_AFTER_RENDER);
        display.update();
        if (bmpDebug) 
            printf("Free heap after display render: %d\n", xPortGetFreeHeapSize());
        // Go to deepsleep after rendering
        vTaskDelay(14000 / portTICK_PERIOD_MS);
        deepsleep();
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
        break;
        // Not handled, ex. HTTP_EVENT_REDIRECT
    default:
        break;
    }
    return ESP_OK;
}

static void http_post(void)
{
    // Show available Dynamic Random Access Memory available after display.init() - Both report same number
    printf("Free heap: %d (After epaper instantiation)\n", xPortGetFreeHeapSize());
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
    
    // TODO POST Send the IP for logging purpouses
    /* char post_data[22];
    uint8_t postsize = sizeof(post_data);
    strlcpy(post_data, "ip=", postsize);
    strlcat(post_data, espIpAddress, postsize); */

    esp_http_client_config_t config = {
        .url = CONFIG_CALE_SCREEN_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 9000,
        .event_handler = _http_event_handler,
        .buffer_size = HTTP_RECEIVE_BUFFER_SIZE
        };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Authentication: Bearer    
    strlcpy(bearerToken, "Bearer: ", sizeof(bearerToken));
    strlcat(bearerToken, CONFIG_CALE_BEARER_TOKEN, sizeof(bearerToken));
    
    //printf("POST data: %s\n%s\n", post_data, bearerToken);

    esp_http_client_set_header(client, "Authorization", bearerToken);
    //esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
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
    //ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
}

/* Event handler for catching RainMaker events */
static void event_handler_rmk(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    printf("EVENT ID:%d\n", event_id);
    display.setCursor(10,10);
    display.setTextColor(EPD_BLACK);
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG, "RainMaker Initialised.");
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG, "RainMaker Claim Started.");
                break;
            case RMAKER_EVENT_CLAIM_SUCCESSFUL:
                ESP_LOGI(TAG, "RainMaker Claim Successful.");
                break;
            case RMAKER_EVENT_CLAIM_FAILED:
                ESP_LOGI(TAG, "RainMaker Claim Failed.");
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Event: %d", event_id);
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_REBOOT:
                ESP_LOGI(TAG, "Rebooting in %d seconds.", *((uint8_t *)event_data));
                break;
            case RMAKER_EVENT_WIFI_RESET:
                ESP_LOGI(TAG, "Wi-Fi credentials reset.");
                display.println("Wi-Fi credentials are cleared");
                display.setCursor(10,30);
                display.println("Will start in WiFi provisioning mode");
                display.update();
                break;
            case RMAKER_EVENT_FACTORY_RESET:
                ESP_LOGI(TAG, "Node reset to factory defaults.");
                break;
            case RMAKER_MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "MQTT Connected.");
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "MQTT Disconnected.");
                break;
            case RMAKER_MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT Published. Msg id: %d.", *((int *)event_data));
                readyToRequestImage = true;
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Common Event: %d", event_id);
        }
    } else {
        ESP_LOGW(TAG, "Invalid event received!");
    }
}

void app_main(void)
{
    //  On  init(true) activates debug (And makes SPI communication slower too)
    display.init();
    display.setRotation(CONFIG_DISPLAY_ROTATION);
    display.setFont(&Ubuntu_M12pt8b);

    esp_err_t err;
    // WiFi log level
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_init()
     */
    app_wifi_init();

    /* Register an event handler to catch RainMaker events */
    //ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &event_handler_rmk, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &event_handler_rmk, NULL));
    
    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Switch");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

     /* Create a device and add the relevant parameters to it */
    epaper_device = esp_rmaker_device_create("CALE-Epaper", ESP_RMAKER_DEVICE_SWITCH, NULL);
    
    esp_rmaker_device_add_cb(epaper_device, write_cb, NULL);
    // Customized minutes till next refresh slider
    esp_rmaker_param_t *min_till_refresh = esp_rmaker_brightness_param_create(DEVICE_PARAM_1, nvs_minutes_till_refresh);
    esp_rmaker_param_add_bounds(min_till_refresh, esp_rmaker_int(10), esp_rmaker_int(360), esp_rmaker_int(1));
    esp_rmaker_device_add_param(epaper_device, min_till_refresh);

    esp_rmaker_param_t *reset_wifi = esp_rmaker_brightness_param_create(DEVICE_PARAM_WIFI_RESET, 0);
    esp_rmaker_param_add_bounds(reset_wifi, esp_rmaker_int(0), esp_rmaker_int(100), esp_rmaker_int(10));
    esp_rmaker_device_add_param(epaper_device, reset_wifi);

    esp_rmaker_node_add_device(node, epaper_device);

    /* Enable OTA */
    /* esp_rmaker_ota_config_t ota_config = {
        .server_cert = ota_server_cert,
    };
    esp_rmaker_ota_enable(&ota_config, OTA_USING_PARAMS); */

    /* Enable timezone service which will be require for setting appropriate timezone
     * from the phone apps for scheduling to work correctly.
     * For more information on the various ways of setting timezone, please check
     * https://rainmaker.espressif.com/docs/time-service.html.
     */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();
    
    /* Uncomment to reset WiFi credentials when there is no Boot button in the ESP32 */
    //esp_rmaker_wifi_reset(1,10);return;

    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    while(true) {
        if (readyToRequestImage){
            // Display.init() and Download the image from www
            http_post();
            readyToRequestImage = false;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

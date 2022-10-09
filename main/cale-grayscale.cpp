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
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_sleep.h"

// Should match your display model (Check WiKi)
// 1 channel SPI epaper displays:

//#include <gdew075T8.h>
//#include <gdew0583t7.h>
//#include <gdew0583z21.h>
#include <gdew042t2Grays.h>
//#include <gdew042t2.h>
//#include <gdew027w3.h>
EpdSpi io;
//Gdew0583z21 display(io);
//Gdew0583T7 display(io);
//Gdew027w3 display(io);
Gdew042t2Grays display(io);
//Gdew042t2 display(io);
//#include <plasticlogic021.h>
//EpdSpi2Cs io;
//PlasticLogic011 display(io);
//PlasticLogic014 display(io);
//PlasticLogic021 display(io);
// Multi-SPI 4 channels EPD only
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
/* // Otherwise you will run out of DRAM very shortly!
#include "wave12i48.h" // Only to use with Edp4Spi IO
Epd4Spi io;
Wave12I48 display(io); */
// PARALLEL Epapers driven by component Epdiy
// NOTE: This needs Epdiy component https://github.com/vroland/epdiy
// Run idf.py menuconfig-> Component Config -> E-Paper driver and select:
// Display type: LILIGO 4.7 ED047TC1
// Board: LILIGO T5-4.7 Epaper
// In the same section Component Config -> ESP32 Specifics -> Enable PSRAM
//#include "parallel/ED047TC1.h"
//Ed047TC1 display;
// BMP debug Mode: Turn false for production since it will make things slower and dump Serial debug
bool bmpDebug = false;

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

uint16_t in_red = 0;   // for depth 24
uint16_t in_green = 0; // for depth 24
uint16_t in_blue = 0;  // for depth 24

/** COLOR Boundaries for gray 
 *  0x00:Black 0x55:DGray  0xAA:LGray  0xFF White  -> only 4 grayscales
 * 
 *  For 8 gray-levels:
 *  0: Black 32: DGray  64: Gray  96: LGRAY C8: SLGRAY DF: Whitish FF: White 
 **/
#ifdef HAS_16_LEVELS_GRAY
    uint8_t wgray_hb = 0xF0; // 240 Near to white
    uint8_t wgray_lb = 0xC9; // 201 Near to super light gray
 
    uint8_t slgray_hb = 0xC9;// 201 Near to whitish
    uint8_t slgray_lb = 0x96;// 150 

    uint8_t lgray_hb = 0x96; // 150 Near to light gray
    uint8_t lgray_lb = 0x64; // 100 Near to gray
    
    uint8_t gray_hb = 0x64;  // 101 Near to light gray
    uint8_t gray_lb = 0x32;  // 50 Near to dark gray
    
    uint8_t dgray_hb = 0x32; // 50 Near to light gray
    uint8_t dgray_lb = 0x19; // Near to super dark

    uint8_t sdgray_hb = 0x19; // 50 Near to dark gray
    uint8_t sdgray_lb = 0x0A; // Near to black

#else
    uint8_t lgray_lb = 0x56; // Near to dark gray
    uint8_t lgray_hb = 0xFA; // Almost white
    uint8_t dgray_lb = 0x01; // Near to black
    uint8_t dgray_hb = 0xA9;
#endif

uint32_t rowSize;
uint32_t rowByteCounter;
uint16_t w;
uint16_t h;
uint8_t bitmask = 0xFF;
uint8_t bitshift;
uint16_t red, green, blue;
bool whitish, color_slgray, color_lgray, color_gray, color_dgray, color_sdgray;
uint16_t drawX = 0;
uint16_t drawY = 0;
uint8_t index24 = 0; // Index for 24 bit
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

uint8_t mono_palette_buffer[max_palette_pixels / 8];         // palette buffer for depth <= 8 b/w
uint8_t whitish_palette_buffer[max_palette_pixels / 8];      // EPD_WHITISH almost white
uint8_t lgray_palette_buffer[max_palette_pixels / 8];        // EPD_LGRAY light
uint8_t dgray_palette_buffer[max_palette_pixels / 8];        // EPD_DGRAY dark
#ifdef HAS_16_LEVELS_GRAY
    uint8_t slgray_palette_buffer[max_palette_pixels / 8];   // EPD_SLGRAY super light
    uint8_t gray_palette_buffer[max_palette_pixels / 8];     // EPD_GRAY
    uint8_t sdgray_palette_buffer[max_palette_pixels / 8];   // EPD_SDGRAY super dark
#endif

uint16_t totalDrawPixels = 0;
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
                ESP_LOGI(TAG, "BMP isPaddingAware:  1 bit depth are 4 bit padded. Wikipedia gave me a lesson.");
            }
            if (((bmp.planes == 1) && ((bmp.format == 0) || (bmp.format == 3))) == false)
            { // uncompressed is handled
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP NOT SUPPORTED: Compressed formats not handled.\nBMP NOT SUPPORTED: Only planes==1, format 0 or 3\n");
            }
            if (bmp.depth > 24 || bmp.depth == 16)
            {
                isSupportedBitmap = false;
                ESP_LOGE(TAG, "BMP DEPTH %d: Only 1, 4, 8 and 24 bits depth are supported.\n", bmp.depth);
            }

            rowSize = (bmp.width * bmp.depth / 8 + 3) & ~3;
            if (bmp.depth < 8)
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

                // Balanced with boundaries defined in global COLORS                               
                    #ifdef HAS_16_LEVELS_GRAY
                      whitish = ((red > 0xF0) && (green > 0xF0) && (blue > 0xF0));
                      color_slgray = (red > slgray_lb && red < slgray_hb) || (green> slgray_lb && green < slgray_hb) || (blue > slgray_lb && blue < slgray_hb);  // EPD_SLGRAY
                      color_lgray  = (red > lgray_lb && red < lgray_hb) || (green> lgray_lb && green < lgray_hb) || (blue > lgray_lb && blue < lgray_hb);        // EPD_LGRAY
                      color_gray   = (red > gray_lb && red < gray_hb) || (green> gray_lb && green < gray_hb) || (blue > gray_lb && blue < gray_hb);              // EPD_GRAY
                      color_dgray  = (red > dgray_lb && red < dgray_hb) || (green > dgray_lb && green < dgray_hb) || (blue > dgray_lb && blue < dgray_hb);       // EPD_DGRAY
                      color_sdgray = (red > sdgray_lb && red < sdgray_hb) || (green > sdgray_lb && green < sdgray_hb) || (blue > sdgray_lb && blue < sdgray_hb);   // EPD_SDGRAY   
                    #else
                      whitish = ((red > 0x80) && (green > 0x80) && (blue > 0x80));
                      color_lgray = (red > lgray_lb && red < lgray_hb) || (green> lgray_lb && green < lgray_hb) || (blue > lgray_lb && blue < lgray_hb); // EPD_LGRAY
                      color_dgray = (red > dgray_lb && red < dgray_hb) || (green > dgray_lb && green < dgray_hb) || (blue > dgray_lb && blue < dgray_hb); // EPD_DGRAY   
                    #endif

                    if (0 == pn % 8) {
                        mono_palette_buffer[pn / 8] = 0;
                        lgray_palette_buffer[pn / 8] = 0;
                        dgray_palette_buffer[pn / 8] = 0;
                        #ifdef HAS_16_LEVELS_GRAY
                        gray_palette_buffer[pn / 8] = 0;
                        slgray_palette_buffer[pn / 8] = 0;
                        sdgray_palette_buffer[pn / 8] = 0;
                        #endif
                        }
                    
                    mono_palette_buffer[pn / 8] |= whitish << pn % 8;                        
                    lgray_palette_buffer[pn / 8] |= color_lgray << pn % 8;
                    dgray_palette_buffer[pn / 8] |= color_dgray << pn % 8;
                    #ifdef HAS_16_LEVELS_GRAY
                      gray_palette_buffer[pn / 8] |= color_gray << pn % 8;                        
                      slgray_palette_buffer[pn / 8] |= color_lgray << pn % 8;
                      sdgray_palette_buffer[pn / 8] |= color_sdgray << pn % 8;
                    #endif

                    if (color_lgray) {
                        printf("pn: %d LGRAY: %x\n",pn,color_lgray);
                    }
                    if (color_dgray) {
                        printf("pn: %d DGRAY: %x\n",pn,color_dgray);
                    }
                    // DEBUG Colors
                    if (bmpDebug)
                        printf("0x00%x%x%x : %x, %x\n", red, green, blue, whitish, color_lgray);
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
                    color_lgray = lgray_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    color_dgray = dgray_palette_buffer[pn / 8] & (0x1 << pn % 8);

                    in_byte <<= bmp.depth;
                    in_bits -= bmp.depth;

                    // Withouth this is coming white first and skips light gray (Research why)
                    if (whitish && !color_lgray)
                    {
                        color = EPD_WHITE;
                    }
                    else if (color_lgray)
                    {
                        color = EPD_LIGHTGREY;
                    }
                    else if (color_dgray)
                    {
                        color = EPD_DARKGREY;
                    }
                    #ifdef HAS_16_LEVELS_GRAY
                        else if (color_slgray)
                        {
                            color = EPD_SLGRAY;
                        }
                        else if (color_gray)
                        {
                            color = EPD_GRAY;
                        }
                        else if (color_sdgray)
                        {
                            color = EPD_SDGRAY;
                        }
                    #endif
                    
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

                    totalDrawPixels++;
                    ++drawX;
                }
            }
            break;

            case 24:
                // index24  3 byte B,G,R counter starts on 1
                ++index24;
                // Convert the 24 bits into 16 bit 565 (Adafruit GFX format)
                switch (index24)
                {
                case 1:
                    in_blue  = in_byte;
                    break;
                case 2:
                    in_green = in_byte;
                    break;
                case 3:
                    in_red   = in_byte;
                    break;
                }
                
                // Every 3rd byte we advance one X
                if (index24 == 3) {
                    if (drawX+1 > bmp.width)
                    {
                        drawX = 0;
                        --drawY;
                    }
                
                    totalDrawPixels++;
                    // With and is also possible but the result is not nice for black/white photography
                    //color = (in_red & in_green & in_blue);
                    color = 0.33 * in_red + 0.34 * in_green + 0.33 * in_blue;
                        // DEBUG: Turn to true
                    if (false && totalDrawPixels<200) {
                        printf("R:%d G:%d B:%d CALC:%d\n", in_red, in_green, in_blue, color);
                    }
                    
                    display.drawPixel(drawX, drawY, color);
                    ++drawX;
                    index24 = 0;
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
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH\nDownload took: %llu ms\nRefresh and go to sleep %d minutes\n", (esp_timer_get_time()-startTime)/1000, CONFIG_DEEPSLEEP_MINUTES_AFTER_RENDER);
        display.update();
        if (bmpDebug) 
            printf("Free heap after display render: %d\n", xPortGetFreeHeapSize());
        // Go to deepsleep after rendering
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        deepsleep();
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
        break;
    }
    return ESP_OK;
}

static void http_post(void)
{
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered. TESTs:
       http://img.cale.es/bmp/fasani/5e8cc4cf03d81  -> 4 bit 2.7 tests
       http://cale.es/img/test/1.bmp                -> vertical line
       http://cale.es/img/test/circle.bmp           -> Circle test
     */
    // POST Send the IP for logging purpouses
    char post_data[22];
    uint8_t postsize = sizeof(post_data);
    strlcpy(post_data, "ip=", postsize);
    strlcat(post_data, espIpAddress, postsize);

    esp_http_client_config_t config = {
        .url = CONFIG_CALE_SCREEN_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
        .buffer_size = HTTP_RECEIVE_BUFFER_SIZE
        };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Authentication: Bearer    
    strlcpy(bearerToken, "Bearer: ", sizeof(bearerToken));
    strlcat(bearerToken, CONFIG_CALE_BEARER_TOKEN, sizeof(bearerToken));
    
    printf("POST data: %s\n%s\n", post_data, bearerToken);

    esp_http_client_set_header(client, "Authorization", bearerToken);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
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
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // WiFi log level
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    
    //  On  init(true) activates debug (And makes SPI communication slower too)
    display.init();
    //display.clearScreen();
    display.setRotation(CONFIG_DISPLAY_ROTATION);
    // Show available Dynamic Random Access Memory available after display.init() - Both report same number
    printf("Free heap: %d (After epaper instantiation)\nDRAM     : %d\n", 
    xPortGetFreeHeapSize(),heap_caps_get_free_size(MALLOC_CAP_8BIT));

    http_post();

    // Just test if Epd works: Compile the demo-epaper.cpp example modifying main/CMakeLists
}

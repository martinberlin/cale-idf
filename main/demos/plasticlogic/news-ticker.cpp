/* The example of ESP-IDF modified by FASANI Corporation ;)
 *
 * This sample code is in the public domain.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_client.h" 
#include "esp_tls.h" 
#include "cJSON.h"

bool debugVerbose = true;
// TinyPICO.com Dotstar or S2 with Neopixel led. Turn down power and set data /clk Gpios
#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12


// Important configuration. The class should match your epaper display model:
#include <plasticlogic021.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic021 display(io);

uint16_t backgroundColor = EPD_WHITE;
uint16_t textColor = EPD_BLACK;
// Adafruit GFX Font selection - - - - - -
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h> // Day, Month
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>  // Last Sync message - Still not fully implemented

// REQUEST Configuration
// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE 1938
#define DEEP_SLEEP_AFTER_RENDER  60
#define NEWS_URL1 "http://newsapi.org/v2/everything?q=Apple&from=2022-04-03&sortBy=popularity&apiKey=156ce2531d734561bdc7db6bb8e82280"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT			 BIT1

static const char *TAG = "JSON";

static int s_retry_num = 0;
uint16_t countDataEventCalls = 0;
uint32_t img_buf_pos = 0;
uint32_t dataLenTotal = 0;
uint64_t startTime = 0;
char *source_buf;    // JSON download buffer, declared in PSRAM in app_main()
// JSON parser helpers
const char* json_title = "title";
const char* json_desc = "description";

extern "C"
{
   void app_main();
}

void deepsleep(uint16_t sleepMinutes){
    esp_deep_sleep(1000000LL * 60 * sleepMinutes);
}

char *JSON_Types(int type) {
	if (type == cJSON_Invalid) return ("cJSON_Invalid");
	if (type == cJSON_False) return ("cJSON_False");
	if (type == cJSON_True) return ("cJSON_True");
	if (type == cJSON_NULL) return ("cJSON_NULL");
	if (type == cJSON_Number) return ("cJSON_Number");
	if (type == cJSON_String) return ("cJSON_String");
	if (type == cJSON_Array) return ("cJSON_Array");
	if (type == cJSON_Object) return ("cJSON_Object");
	if (type == cJSON_Raw) return ("cJSON_Raw");
	return NULL;
}

void JSON_Parse(const cJSON * const root) {
	//ESP_LOGI(TAG, "root->type=%s", JSON_Types(root->type));
	cJSON *current_element = NULL;
	//ESP_LOGI(TAG, "roo->child=%p", root->child);
	//ESP_LOGI(TAG, "roo->next =%p", root->next);
	cJSON_ArrayForEach(current_element, root) {
		//ESP_LOGI(TAG, "type=%s", JSON_Types(current_element->type));
		//ESP_LOGI(TAG, "current_element->string=%p", current_element->string);
		if (current_element->string) {
			const char* string = current_element->string;
			ESP_LOGI(TAG, "[%s]", string);
		}
		if (cJSON_IsInvalid(current_element)) {
			ESP_LOGI(TAG, "Invalid");
		} else if (cJSON_IsFalse(current_element)) {
			ESP_LOGI(TAG, "False");
		} else if (cJSON_IsTrue(current_element)) {
			ESP_LOGI(TAG, "True");
		} else if (cJSON_IsNull(current_element)) {
			ESP_LOGI(TAG, "Null");
		} else if (cJSON_IsNumber(current_element)) {
			int valueint = current_element->valueint;
			double valuedouble = current_element->valuedouble;
			ESP_LOGI(TAG, "int=%d double=%f", valueint, valuedouble);
		} else if (cJSON_IsString(current_element)) {
			const char* valuestring = current_element->valuestring;
			//const char* json_title = "title";
			if (strcmp(json_title, current_element->string) == 0) {
				display.clearScreen();
				display.setCursor(1, 18);
				display.setFont(&Ubuntu_M12pt8b);
				display.print(valuestring);
			}
			if (strcmp(json_desc, current_element->string) == 0) {
				display.setFont(&Ubuntu_M8pt8b);
				display.println("");
				display.print(valuestring);
				display.update();
				vTaskDelay(2000 / portTICK_PERIOD_MS);
			}
			ESP_LOGI(TAG, "%s", valuestring);
		} else if (cJSON_IsArray(current_element)) {
			//ESP_LOGI(TAG, "Array");
			JSON_Parse(current_element);
		} else if (cJSON_IsObject(current_element)) {
			//ESP_LOGI(TAG, "Object");
			JSON_Parse(current_element);
		} else if (cJSON_IsRaw(current_element)) {
			ESP_LOGI(TAG, "Raw(Not support)");
		}
	}
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch(evt->event_id) {
		case HTTP_EVENT_ERROR:
			ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
			break;
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
			break;
		case HTTP_EVENT_HEADER_SENT:
			ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
			break;
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
			break;
		case HTTP_EVENT_ON_DATA:
            ++countDataEventCalls;
            #if DEBUG_VERBOSE
            if (countDataEventCalls%2==0) {
                ESP_LOGI(TAG, "%d len:%d\n", countDataEventCalls, evt->data_len);
            }
            #endif
            dataLenTotal += evt->data_len;
            if (countDataEventCalls == 1) startTime = esp_timer_get_time();

            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Append received data into source_buf -> DIES here, why?
                memcpy(&source_buf[img_buf_pos], evt->data, evt->data_len);
                //ESP_LOG_BUFFER_HEX(TAG, evt->data, evt->data_len);
                img_buf_pos += evt->data_len;
            }
			break;


		case HTTP_EVENT_ON_FINISH:
			ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH status: %d", esp_http_client_get_status_code(evt->client));

            if (esp_http_client_get_status_code(evt->client) == 200) {
                cJSON *root = cJSON_Parse(source_buf);
                JSON_Parse(root);
                cJSON_Delete(root);
                
				printf("download + parse mess took %lld ms\n", (esp_timer_get_time()-startTime)/1000);
				deepsleep(DEEP_SLEEP_AFTER_RENDER);
            } else {
            	printf("HTTP on finish got status code: %d\n", esp_http_client_get_status_code(evt->client));
            }
			break;

		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
			break;
	}
	return ESP_OK;
}


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
    sprintf(reinterpret_cast<char *>(wifi_config.sta.ssid), CONFIG_ESP_WIFI_SSID);
    sprintf(reinterpret_cast<char *>(wifi_config.sta.password), CONFIG_ESP_WIFI_PASSWORD);
    wifi_config.sta.pmf_cfg.capable = true;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
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
				ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_ESP_WIFI_SSID);
		} else if (bits & WIFI_FAIL_BIT) {
				ESP_LOGI(TAG, "Failed to connect to SSID:%s", CONFIG_ESP_WIFI_SSID);
		} else {
				ESP_LOGE(TAG, "UNEXPECTED EVENT");
		}

		/* The event will not be processed after unregister */
		ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
		ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
		vEventGroupDelete(s_wifi_event_group);
}

void http_client(char * url)
{
	
	esp_http_client_config_t config = {
		.url = url,
		.event_handler = _http_event_handler,
        .buffer_size = HTTP_RECEIVE_BUFFER_SIZE,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	// GET
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
				esp_http_client_get_status_code(client),
				esp_http_client_get_content_length(client));
	} else {
		ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
}


void app_main()
{
    // Turn off neopixel to keep consumption to the minimum
    gpio_set_direction((gpio_num_t)DOTSTAR_PWR, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode((gpio_num_t)DOTSTAR_CLK, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode((gpio_num_t)DOTSTAR_DATA, GPIO_PULLDOWN_ONLY);
    gpio_set_level((gpio_num_t)DOTSTAR_PWR, 0);
	// JSON big buffer 
	source_buf = (char *)heap_caps_malloc(100000, MALLOC_CAP_SPIRAM);

	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	display.init();
	display.fillScreen(backgroundColor);
	display.setTextColor(textColor);

	// WiFi log level set only to Error otherwise outputs too much
	esp_log_level_set("wifi", ESP_LOG_ERROR);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();

	// get JSON
	char url[164];
	sprintf(url, "%s", NEWS_URL1);
	ESP_LOGI(TAG, "url=%s",url);
	http_client(url); 

}

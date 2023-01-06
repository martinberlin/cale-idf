/**
 * This is a demo to be used with Good Display 2.7 touch epaper 
 */ 

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"
#include "driver/gpio.h"

#include "goodisplay/gdey027T91.h"

// Non volatile storage
#include "nvs.h"
#include "nvs_flash.h"
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

static const char *TAG = "SWITCH";
#define DEVICE_PARAM_1 "SWITCH"
#define DEVICE_PARAM_WIFI_RESET "Turn slider to 100 to reset WiFi"
bool switch_state = false; // false = OFF
bool ready_mqtt = false;
// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);
EpdSpi io;
Gdey027T91 display(io);

esp_rmaker_device_t *epaper_device;
// Values that will be stored in NVS - defaults here
nvs_handle_t nvs_h;

// Only debugging:
//#define DEBUG_COUNT_TOUCH

// Relay ON (high) / OFF
#define GPIO_RELAY 25
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
uint8_t display_rotation = 0;

extern "C"
{
   void app_main();
}

// Some GFX constants
uint16_t blockWidth = 42;
uint16_t blockHeight = display.height()/4;
uint16_t lineSpacing = 18;
uint16_t circleColor = EPD_BLACK;
uint16_t circleRadio = 10;
uint16_t selectTextColor  = EPD_WHITE;
uint16_t selectBackground = EPD_BLACK;
template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void draw_centered_text(const GFXfont *font, char * text, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    // Draw external boundary where text needs to be centered in the middle
    printf("drawRect x:%d y:%d w:%d h:%d\n\n", x, y, w, h);
    display.drawRect(x, y, w, h, EPD_DARKGREY);

    display.setFont(font);
    int16_t text_x = 0;
    int16_t text_y = 0;
    uint16_t text_w = 0;
    uint16_t text_h = 0;

    display.getTextBounds(text, x, y, &text_x, &text_y, &text_w, &text_h);
    printf("text_x:%d y:%d w:%d h:%d\n\n", text_x,text_y,text_w,text_h);
    //display.drawRect(text_x, text_y, text_w, text_h, EPD_BLACK); // text boundaries

    if (text_w > w) {
        printf("W: Text width out of bounds");
    }
    if (text_h > h) {
        printf("W: Text height out of bounds");
    }
    // Calculate the middle position
    text_x += (w-text_w)/2;

    uint ty = (h/2)+y+(text_h/2);

    printf("setCusor x:%d y:%d\n", text_x, ty);
    display.setCursor(text_x, ty);
    display.print(text);
}


void drawUXandSwitch(){
  uint16_t dw = display.width();
  uint16_t dh = display.height();
  uint8_t  sw = 20;
  uint8_t  sh = 50;
  uint8_t  keyw = 16;
  uint8_t  keyh = 20;
  display.fillScreen(EPD_WHITE);
  display.drawRoundRect(dw/2-sw/2, dh/2-sh/2, sw, sh, 4, EPD_BLACK);

  // OFF position
  if (!switch_state) {
    display.fillRoundRect(dw/2-keyw/2, dh/2, keyw, keyh, 5, EPD_BLACK);
    gpio_set_level((gpio_num_t)GPIO_RELAY, 0); // OFF
  } else {
    display.fillRoundRect(dw/2-keyw/2, dh/2-keyh, keyw, keyh, 5, EPD_BLACK);
    gpio_set_level((gpio_num_t)GPIO_RELAY, 1); // ON
  }
  
  char * label = (switch_state) ? (char *)"ON" : (char *)"OFF";
  draw_centered_text(&Ubuntu_M8pt8b, label, dw/2-22, dh/2-sh, 40, 20);
  display.update();
  // It does not work correctly with partial update leaves last position gray
  //display.updateWindow(dw/2-40, dh/2-keyh-40, 100, 86);
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

    } else if (strcmp(param_name, DEVICE_PARAM_1) == 0) {
      if (val.val.b) {
          switch_state = true;
        } else {
          switch_state = false;
        }
        drawUXandSwitch();
        ESP_LOGI(TAG, "%d for %s-%s",
                (int)val.val.i, device_name, param_name);

        nvs_handle_t my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } 
        nvs_set_u16(my_handle, "switch", (uint16_t) val.val.i);

        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "NVS Failed to store %d\n" : "NVS Stored %d\n", (int)val.val.i);
        nvs_close(my_handle);    

    } else if (strcmp(param_name, DEVICE_PARAM_WIFI_RESET) == 0) {
        ESP_LOGI(TAG, "%d for %s-%s",
               (int) val.val.i, device_name, param_name);
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

/* Event handler for catching RainMaker events */
static void event_handler_rmk(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    printf("EVENT ID:%d\n", (int)event_id);
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
                ESP_LOGW(TAG, "Unhandled RainMaker Event: %d", (int)event_id);
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
                ready_mqtt = true;
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Common Event: %d", (int)event_id);
        }
    } else {
        ESP_LOGW(TAG, "Invalid event received!");
    }
}

// Handle touch
uint16_t t_counter = 0;

void touchEvent(TPoint p, TEvent e)
{
  #if defined(DEBUG_COUNT_TOUCH) && DEBUG_COUNT_TOUCH==1
    ++t_counter;
    printf("e %x %d  ",e,t_counter); // Working
  #endif

  if (e != TEvent::Tap && e != TEvent::DragStart && e != TEvent::DragMove && e != TEvent::DragEnd)
    return;

  switch_state = !switch_state;
  drawUXandSwitch();
  //printf("state:%d\n", (int)switch_state);
}

void app_main(void)
{
  printf("CalEPD version: %s\n", CALEPD_VERSION);
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
    epaper_device = esp_rmaker_device_create("EPD Switch", ESP_RMAKER_DEVICE_SWITCH, NULL);
    
    esp_rmaker_device_add_cb(epaper_device, write_cb, NULL);
    // Customized minutes till next refresh slider
    esp_rmaker_param_t *switch_param = esp_rmaker_power_param_create(DEVICE_PARAM_1, switch_state);
    esp_rmaker_device_add_param(epaper_device, switch_param);

    esp_rmaker_param_t *reset_wifi = esp_rmaker_brightness_param_create(DEVICE_PARAM_WIFI_RESET, 0);
    esp_rmaker_param_add_bounds(reset_wifi, esp_rmaker_int(0), esp_rmaker_int(100), esp_rmaker_int(10));
    esp_rmaker_device_add_param(epaper_device, reset_wifi);

    esp_rmaker_node_add_device(node, epaper_device);

   //Initialize GPIOs direction & initial states
    gpio_set_direction((gpio_num_t)GPIO_RELAY, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)GPIO_RELAY, 0); // OFF

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
      
   // Test Epd class
   display.init(false);
   //display.setFont(&Ubuntu_M8pt8b);
   
   printf("display.colors_supported:%d\n", display.colors_supported);  
   display.setRotation(2);
   display.update();
   display.setFont(&Ubuntu_M8pt8b);
   display.setTextColor(EPD_BLACK);
   drawUXandSwitch();
   
   // Instantiate touch. Important pass here the 3 required variables including display width and height
   ts.begin(FT6X36_DEFAULT_THRESHOLD, display.width(), display.height());
   ts.setRotation(display.getRotation());
   ts.registerTouchHandler(touchEvent);
  
    for (;;) {
        ts.loop();
      }
      
}

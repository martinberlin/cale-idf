/****************************************************************************************************************
*
* This demo showcases BLE GATT server. It can send adv data, be connected by client CALE.es (Only Chrome example)
* ATTENTION: Needs menuconfig -> Components -> Bluetooth (Active)
*****************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include <math.h> // round + pow
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
// Update this with your EPD class
#include <gdew042t2Grays.h>
EpdSpi io;
Gdew042t2Grays display(io);


// Fonts
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>

uint64_t USEC = 1000000;
int cursor_x = 0;
int cursor_y = 0;
uint8_t powered_by = 0;

// JPG decoder from @bitbank2
#include "JPEGDEC.h"
JPEGDEC jpeg;

// Dither space allocation
uint8_t * dither_space;
uint8_t *source_buf;      // JPG receive buffer
uint32_t img_buf_pos = 0;
uint8_t gamme_curve[256]; // Internal array for gamma grayscale
// Nice test values: 0.9 1.2 1.4 higher and is too bright
double gamma_value = 1.4;
// Timers
#include "esp_timer.h"
uint32_t time_decomp = 0;
uint32_t time_render = 0;
uint32_t time_receive = 0;
uint64_t start_time = 0;
uint32_t received_length = 0;

// Draws a progress bar when downloading (Just a demo: is faster without it)
// And also writes in the same framebuffer as the image
#define DOWNLOAD_PROGRESS_BAR 0
uint8_t progressBarHeight = 10;


#define GATTS_TAG "BLE"

///Declare the static function
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*                           LSB <----------------------> MSB */
// 0000180d-0000-1000-8000-00805f9b34fb
static uint16_t BLE_SERVICE_UUID = 0x180d; // Short Service UUID

// BOTH SAME just for a demo
static uint16_t GATTS_CHAR_UUID_TEST_A = 0x180d;
#define GATTS_NUM_HANDLE_TEST_A     4

#define TEST_DEVICE_NAME            "BLE_JPEG_042"
#define TEST_MANUFACTURER_DATA_LEN  17

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 16

#define PREPARE_BUF_MAX_SIZE 1024

static uint8_t char1_str[] = {0x18,0x0d};
static esp_gatt_char_prop_t a_property = 0;

static esp_attr_value_t gatts_char1_uuid =
{
    .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
    .attr_len     = sizeof(char1_str),
    .attr_value   = char1_str,
};

uint16_t received_events = 0;

static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        0x02, 0x01, 0x06,
        0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
};
static uint8_t raw_scan_rsp_data[] = {
        0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
        0x45, 0x4d, 0x4f
};
#else
// TODO: Research how to use this and RAW adv option
static uint8_t adv_service_uuid128[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT 
   .gatts_if state: Not get the gatt_if, so initial is ESP_GATT_IF_NONE
*/
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    }
};

typedef struct {
    uint8_t *prepare_buf;
    int     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t a_prepare_write_env;

extern "C"
{
    void app_main();
}

void delay_ms(uint32_t period_ms) {
    vTaskDelay(period_ms/portTICK_PERIOD_MS);
}

void progressBar(long processed, long total)
{
  int percentage = round(processed * display.width() / total);
  display.fillRect(0, 0, percentage, progressBarHeight, EPD_BLACK);
}

//====================================================================================
// This program contains support functions to render the Jpeg images
//
// JPEGDEC fucntions by Bitbank
// Refactored by @martinberlin for EPDiy as a Jpeg receive and render example
//====================================================================================

// color Borders: This is a crap selection that could be made much better
uint8_t white2lg = 150;
uint8_t lg2dg = 100;
uint8_t dg2black = 50;

/**
 * @brief Convert 255 (WHITE) gradient till 0 (BLACK) into 4 grays
 */
uint16_t colorTo4Gray(uint8_t color) {
    if (color > white2lg) {
        return EPD_WHITE;
    }
    if (color > lg2dg && color < white2lg) {
        //printf("LG ");
        return EPD_LIGHTGREY;
    }
    if (color > dg2black && color < lg2dg) {
        return EPD_DARKGREY;
    }
   
    return EPD_BLACK;
}

int JPEGDraw4Bits(JPEGDRAW *pDraw)
{
  uint32_t render_start = esp_timer_get_time();
  for (int16_t xx = 0; xx < pDraw->iWidth; xx+=4) {
      for (int16_t yy = 0; yy < pDraw->iHeight; yy++) {
        uint16_t col = pDraw->pPixels[ (xx + (yy * pDraw->iWidth)) >>2 ];
      
        uint8_t col1 = col & 0xf;
        uint8_t col2 = (col >> 4) & 0xf;
        uint8_t col3 = (col >> 8) & 0xf;
        uint8_t col4 = (col >> 12) & 0xf;
        // Larry: It could be upper/lower nibble are reversed. Yes he was right:
        display.drawPixel(pDraw->x + xx +1, pDraw->y + yy, colorTo4Gray(gamme_curve[col1*16])); // 0
        display.drawPixel(pDraw->x + xx , pDraw->y + yy, colorTo4Gray(gamme_curve[col2*16]));   // 1
        display.drawPixel(pDraw->x + xx + 3, pDraw->y + yy, colorTo4Gray(gamme_curve[col3*16]));// 2
        display.drawPixel(pDraw->x + xx + 2, pDraw->y + yy, colorTo4Gray(gamme_curve[col4*16]));// 3

        /* if (yy==0 && mcu_count==0) {
          printf("1.%d %d %d %d ",col1,col2,col3,col4);
        } */
      }
    }

  time_render += (esp_timer_get_time() - render_start) / 1000;
  return 1;
}

//====================================================================================
//   This function opens source_buf Jpeg image file and primes the decoder
//====================================================================================
int decodeJpeg(uint8_t *source_buf, int xpos, int ypos) {
  uint32_t decode_start = esp_timer_get_time();

  if (jpeg.openRAM(source_buf, img_buf_pos, JPEGDraw4Bits)) {

    jpeg.setPixelType(FOUR_BIT_DITHERED);
    
    if (jpeg.decodeDither(dither_space, 0))
    //if (jpeg.decode(0,0,0))
      {
        time_decomp = (esp_timer_get_time() - decode_start)/1000 - time_render;
        ESP_LOGI("decode", "%d ms - %dx%d", time_decomp, jpeg.getWidth(), jpeg.getHeight());
      } else {
        ESP_LOGE("jpeg.decode", "Failed with error: %d", jpeg.getLastError());
      }

  } else {
    ESP_LOGE("jpeg.openRAM", "Failed with error: %d", jpeg.getLastError());
  }
  jpeg.close();
  
  return 1;
}

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

void reset_variables() {
    img_buf_pos = 0;
    time_decomp = 0;
    time_render = 0;
    time_receive = 0;
    start_time = 0;
    received_events = 0;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#else
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#endif
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
        } else {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp){
        if (param->write.is_prep){
            if (prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            } else {
                if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_OFFSET;
                } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_ATTR_LEN;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TAG, "Send response error\n");
            }
            free(gatt_rsp);
            if (status != ESP_GATT_OK){
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        }else{
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
        esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);

    }else{
        ESP_LOGI(GATTS_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:
    {
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        // (uint8_t *) &VAR   or short BLE_SERVICE_UUID
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = BLE_SERVICE_UUID;

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
        if (set_dev_name_ret){
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
#ifdef CONFIG_SET_RAW_ADV_DATA
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret){
            ESP_LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret){
            ESP_LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
#else
        //config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        //config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

#endif
        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
        break;
    }
    case ESP_GATTS_READ_EVT: 
    {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0xde;
        rsp.attr_value.value[1] = 0xed;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        received_events++;
        bool is_short_cmd = false;
        if (received_events == 2) start_time = esp_timer_get_time();

        //ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        
        if (!param->write.is_prep) {
            // content-length: 4 bytes uint32
            if (received_events == 1 && param->write.len == 5 && param->write.value[0] == 0x01) {
                is_short_cmd = true;
                received_length = 
                param->write.value[1] + 
                (param->write.value[2] << 8) +
                (param->write.value[3] << 16) +
                (param->write.value[4] << 24);
                ESP_LOGI(GATTS_TAG, "0x01 content-lenght received:%d", received_length);

                esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            }
            // 0x09 EOF
            if (param->write.len == 1 && param->write.value[0] == 0x09) {
                is_short_cmd = true;
                // Decode & render
                ESP_LOGI(GATTS_TAG, "0x09 EOF received");
                // Decode & render
                time_receive = (esp_timer_get_time()-start_time)/1000;
                decodeJpeg(source_buf, 0, 0);
                ESP_LOGI("ble-rec", "%d ms - download", time_receive);
                ESP_LOGI("render", "%d ms - copying pix", time_render);

                printf("Free heap:%d after JPG decode\n",xPortGetFreeHeapSize());
                // RESET Pointers
                reset_variables();

                display.update();
                break;
            }
            if (!is_short_cmd) {
                // Receive data: Append bytes into source_buf
                memcpy(&source_buf[img_buf_pos], param->write.value, param->write.len);
                img_buf_pos += param->write.len;

                ESP_LOGI(GATTS_TAG, "WRITE_EVT. Len:%d total:%d", param->write.len, img_buf_pos);
            }

            // TO Debug received bytes
            /* if (received_events<3) {
                esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            } */
            #if DOWNLOAD_PROGRESS_BAR
            if (received_events%10 == 0) {
                progressBar(img_buf_pos, received_length);
            }
            #endif

        }
        example_write_event_env(gatts_if, &a_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGE(GATTS_TAG, "ESP_GATTS_EXEC_WRITE_EVT not implemented. Do not send chunks > 896 bytes");
    
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_UNREG_EVT");
        break;
    case ESP_GATTS_CREATE_EVT:
    {
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        // Service uuid or Charasteristic?
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A; // GATTS_CHAR_UUID_TEST_A

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
        a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, 
                                                        &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        a_property,
                                                        &gatts_char1_uuid, NULL);
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    }
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
    {
        break;
    }
    case ESP_GATTS_ADD_CHAR_EVT: 
    {
        uint16_t length = 0;
        const uint8_t *prf_char;

        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
                param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
        if (get_attr_ret == ESP_FAIL){
            ESP_LOGE(GATTS_TAG, "ILLEGAL HANDLE");
        }

        ESP_LOGI(GATTS_TAG, "the gatts demo char length = %x\n", length);
        for(int i = 0; i < length; i++){
            ESP_LOGI(GATTS_TAG, "prf_char[%x] =%x\n",i,prf_char[i]);
        }
        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].descr_uuid,
                                                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (add_descr_ret){
            ESP_LOGE(GATTS_TAG, "add char descr failed, error code =%x", add_descr_ret);
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
    {
        gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    }
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT: {
        /* display.fillScreen(EPD_WHITE);
        display.update(); */

        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 1000;    // timeout = 400*10ms = 4000ms
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        reset_variables();

        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void write_text(int x, int y, char* text) {
    display.setCursor(x, y);
    display.print(text);
}

// Flag to know that we've synced the hour with timeQuery request
int16_t nvs_boots = 0;

void app_main(void)
{
    esp_err_t ret;
    display.init(0);
    display.setMonoMode(false); // Use 4 Grays
    display.setFont(&Ubuntu_M16pt8b);
    
    printf("Free heap:%d\n",xPortGetFreeHeapSize());

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    nvs_handle_t my_handle;
    ret = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(GATTS_TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
    } 
    // Read stored
    nvs_get_i16(my_handle, "boots", &nvs_boots);
    ESP_LOGI(GATTS_TAG, "-> NVS Boot count: %d", nvs_boots);
    nvs_boots++;
    // Set new value
    nvs_set_i16(my_handle, "boots", nvs_boots);
    
    if (nvs_boots%2 == 0) {
        // One boot yes, one no A - B test do something different
    }
    //write_text(cursor_x, cursor_y, (char*)"Use Chrome only. Recommended: cale.es");

    int cursor_x = 60;
    int cursor_y = display.height() - 30;
    write_text(cursor_x, cursor_y, (char*)"BLE initialized");
    cursor_y-= 44;
    display.fillRect(10,display.height()-40,10,10,EPD_BLACK);
    display.fillRect(20,display.height()-40,10,10,EPD_DARKGREY);
    display.fillRect(30,display.height()-40,10,10,EPD_LIGHTGREY);
    display.update();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    cursor_y += 30;
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        write_text(cursor_x, cursor_y, (char*)"Initializing BLE controller failed");
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_B_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    
    printf("Free heap:%d after BLE init\n",xPortGetFreeHeapSize());
    cursor_y = 10;

        double gammaCorrection = 1.0 / gamma_value;
    for (int gray_value =0; gray_value<256;gray_value++) {
        gamme_curve[gray_value]= round (255*pow(gray_value/255.0, gammaCorrection));
    }
    
    dither_space = (uint8_t *)heap_caps_malloc(display.width() *16, MALLOC_CAP_SPIRAM);
    if (dither_space == NULL) {
        ESP_LOGE("main", "Initial alloc ditherSpace failed!");
    }

    // Should be big enough to allocate the JPEG file size, width * height should suffice
    source_buf = (uint8_t *)heap_caps_malloc(display.width() * display.height()/2, MALLOC_CAP_SPIRAM);
    if (source_buf == NULL) {
        ESP_LOGE("main", "Initial alloc source_buf failed!");
    }

    printf("Free heap:%d after JPG buffer\n",xPortGetFreeHeapSize());
}

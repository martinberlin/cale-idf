/* Aus BOSCH-Anleitung unter https://github.com/BoschSensortec/BME280_driver */
// Example from              https://github.com/SFeli/ESP32_BME280_IDF
#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "bmp280.h"

struct bmp280_dev bmp;

static const char *TAG = "i2c test";
#define SDA_GPIO 7
#define SCL_GPIO 15
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY
#define RTOS_DELAY_1SEC          ( 1 * 1000 / portTICK_PERIOD_MS)
uint8_t settings_sel;
int8_t bmestatus = BMP280_OK;

// setup i2c master
static esp_err_t i2c_master_init() {
    i2c_config_t i2c_config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = SDA_GPIO,
            .scl_io_num = SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 100000,
            .clk_flags = 0
    };
    i2c_param_config(I2C_NUM_0, &i2c_config);
    return i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0);
}

void delay_ms(uint32_t period_ms) {
    vTaskDelay(period_ms / portTICK_PERIOD_MS);
    //ets_delay_us(period_ms * 1000);
}

int8_t i2c_reg_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t length) {
    int8_t iError;
    int8_t i2c_addr = dev_id;
    esp_err_t esp_err;
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, reg_addr, true);
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (i2c_addr << 1) | I2C_MASTER_READ, true);
    if (length > 1) {
        i2c_master_read(cmd_handle, reg_data, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd_handle, reg_data + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);
    esp_err = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
 
    if (esp_err == ESP_OK) {
        iError = 0;
    } else {
        iError = -1;
    }
    i2c_cmd_link_delete(cmd_handle);
    return iError;
}
 
int8_t i2c_reg_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t length) {
    int8_t i2c_addr = dev_id;
    int8_t iError;
    esp_err_t esp_err;
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, reg_addr, true);
    i2c_master_write(cmd_handle, reg_data, length,true);
    i2c_master_stop(cmd_handle);
    esp_err = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    if (esp_err == ESP_OK) {
        iError = 0;
    } else {
        iError = -1;
    }
    i2c_cmd_link_delete(cmd_handle);
    return iError;
}
void print_rslt(const char api_name[], int8_t rslt) {
 
    if (rslt != BMP280_OK) {
        printf("%s\t", api_name);
        if (rslt == BMP280_E_NULL_PTR) {
            printf("Error [%d] : Null pointer error\r\n", rslt);
        } else if (rslt == BMP280_E_DEV_NOT_FOUND) {
            printf("Error [%d] : Device not found\r\n", rslt);
        } else if (rslt == BMP280_E_INVALID_LEN) {
            printf("Error [%d] : Invalid Lenght\r\n", rslt);
        } else if (rslt == BMP280_E_COMM_FAIL) {
            printf("Error [%d] : Bus communication failed\r\n", rslt);
        } else if (rslt == BMP280_E_INVALID_MODE) {
            printf("Error [%d] : BMP280_E_INVALID_MODE\r\n", rslt);
        } else {
            /* For more error codes refer "*_defs.h" */
            printf("Error [%d] : Unknown error code\r\n", rslt);
        }
    }else{
        printf("%s\t", api_name);
        printf(" BMP280 status [%d]\n",rslt);
    }
}


void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
        // i2c init & scan
    if (i2c_master_init() != ESP_OK)
        ESP_LOGE(TAG, "i2c init failed\n");

     printf("i2c scan: \n");
     for (uint8_t i = 1; i < 127; i++)
     {
        int ret;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    
        if (ret == ESP_OK)
        {
            printf("Found device at: 0x%2x\n", i);
        }
    }

    bmp.dev_id = BMP280_I2C_ADDR_SEC;
    bmp.intf = BMP280_I2C_INTF;
    bmp.read = i2c_reg_read;
    bmp.write = i2c_reg_write;
    bmp.delay_ms = delay_ms;

    bmestatus = bmp280_init(&bmp);
    print_rslt("bmp280_init status  ", bmestatus);
}

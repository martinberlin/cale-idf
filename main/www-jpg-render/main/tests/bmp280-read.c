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

static const char *TAG = "i2c test";
//ESP32-S3
#define SDA_GPIO 7
#define SCL_GPIO 15
//ESP32 test
/* 
#define SDA_GPIO 25
#define SCL_GPIO 26 */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY
#define RTOS_DELAY_1SEC          ( 1 * 1000 / portTICK_PERIOD_MS)
uint8_t settings_sel;
int8_t bmestatus = BMP280_OK;

int8_t rslt;
struct bmp280_dev bmp;
struct bmp280_config conf;
struct bmp280_uncomp_data ucomp_data;
int32_t temp32;
double temp;

// setup i2c master
static esp_err_t i2c_master_init() {
    i2c_config_t i2c_config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = SDA_GPIO,
            .scl_io_num = SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 10000,
            .clk_flags = 0
    };
    i2c_param_config(I2C_NUM_0, &i2c_config);
    return i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0);
}

void delay_ms(uint32_t period_ms) {
    ets_delay_us(period_ms * 1000);
}

int8_t i2c_reg_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t length) {
    //printf("reg_addr %x, reg_data %d, len %d\n", reg_addr, *(reg_data), length);
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

    bmp.dev_id = BMP280_I2C_ADDR_PRIM;
    bmp.intf = BMP280_I2C_INTF;
    bmp.read = i2c_reg_read;
    bmp.write = i2c_reg_write;
    bmp.delay_ms = delay_ms;

    bmestatus = bmp280_init(&bmp);
    printf("dev_id addr %x\n", BMP280_I2C_ADDR_SEC);
    print_rslt("bmp280_init status  ", bmestatus);


    /* Always read the current settings before writing, especially when
     * all the configuration is not modified
     */
    rslt = bmp280_get_config(&conf, &bmp);
    print_rslt(" bmp280_get_config status", rslt);

    /* configuring the temperature oversampling, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP280_FILTER_COEFF_2;

    /* Temperature oversampling set at 4x */
    conf.os_temp = BMP280_OS_4X;

    /* Pressure over sampling none (disabling pressure measurement with BMP280_OS_NONE) */
    conf.os_pres = BMP280_OS_1X;

    /* Setting the output data rate as 1HZ(1000ms) */
    conf.odr = BMP280_ODR_1000_MS;
    rslt = bmp280_set_config(&conf, &bmp);
    print_rslt(" bmp280_set_config status", rslt);

    /* Always set the power mode after setting the configuration */
    rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);
    print_rslt(" bmp280_set_power_mode status", rslt);


    while (true)
    {
        /* Reading the raw data from sensor */
        rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

        /* Getting the 32 bit compensated temperature */
        rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);

        /* Getting the compensated temperature as floating point value */
        rslt = bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);

        uint32_t pressure = 0;
        rslt = bmp280_get_comp_pres_32bit(&pressure, ucomp_data.uncomp_press, &bmp);
        /* Convert PA into hPA hecto pascal */
        pressure = pressure/100;
        printf("Temp: %f | Pressure: %d\r\n", temp, pressure);

        /* Sleep time between measurements = BMP280_ODR_1000_MS */
        bmp.delay_ms(2000);
    }

}

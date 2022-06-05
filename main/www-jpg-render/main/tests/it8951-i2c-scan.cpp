#define I2C_SDA_PIN 7
#define I2C_SCL_PIN 15
#define I2C_MASTER_FREQ_HZ 200000                     /*!< I2C master clock frequency */
#define I2C_SCLK_SRC_FLAG_FOR_NOMAL       (0)         /*!< Any one clock source that is available for the specified frequency may be choosen*/

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c test";

extern "C"
{
    void app_main();
}

// setup i2c master
static esp_err_t i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    i2c_param_config(I2C_NUM_0, &conf);
    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void app_main()
{
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
}

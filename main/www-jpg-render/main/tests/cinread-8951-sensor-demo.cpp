/* Aus BOSCH-Anleitung unter https://github.com/BoschSensortec/BME280_driver */
// Example from              https://github.com/SFeli/ESP32_BME280_IDF

// RESEARCH FOR SPI HAT Cinread:  https://github.com/martinberlin/H-cinread-it895
// Goal: Read temperature from Bosch sensor and print it on the epaper display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
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
// Include Adafruit big font
#include <Fonts/ubuntu/Ubuntu_M48pt8b.h>

static const char *TAG = "Cinread PCB";
// ESP32-S3 I2C
#define SDA_GPIO 7
#define SCL_GPIO 15
// Display configuration
#define EPD_WIDTH  1200
#define EPD_HEIGHT 825

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

extern "C" {
    void app_main();
}
// setup i2c master
static esp_err_t i2c_master_init() {
    i2c_config_t i2c_config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = SDA_GPIO,
            .scl_io_num = SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .clk_flags = 0
    };
	i2c_config.master.clk_speed = 10000;

    i2c_param_config(I2C_NUM_0, &i2c_config);
    return i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0);
}

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_IT8951   _panel_instance;
  lgfx::Bus_SPI       _bus_instance; 
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

// SPIバスの設定
      cfg.spi_host = SPI2_HOST;     // 使用するSPIを選択  (VSPI_HOST or HSPI_HOST)
      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.spi_3wire  = false;        // 受信をMOSI IMPORTANT use it on false to read from MISO!
      cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)   使用するDMAチャンネルを設定 (0=DMA不使用)
      cfg.pin_sclk = CONFIG_EINK_SPI_CLK;            // SPIのSCLKピン番号を設定
      cfg.pin_mosi = CONFIG_EINK_SPI_MOSI;           // SPIのMOSIピン番号を設定
      cfg.pin_miso = CONFIG_EINK_SPI_MISO;           // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = -1;            // SPIのD/Cピン番号を設定  (-1 = disable)
 
      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs           =    CONFIG_EINK_SPI_CS;  // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    -1;
      cfg.pin_busy         =    CONFIG_EINK_BUSY;    // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.memory_width     =   EPD_WIDTH;  // ドライバICがサポートしている最大の幅
      cfg.memory_height    =   EPD_HEIGHT;  // ドライバICがサポートしている最大の高さ
      cfg.panel_width      =   EPD_WIDTH;  // 実際に表示可能な幅
      cfg.panel_height     =   EPD_HEIGHT;  // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.invert           =  true;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // データ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};
LGFX display;

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
    /* configuring the temperature oversampling, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP280_FILTER_COEFF_2;
    /* Temperature oversampling set at 4x */
    conf.os_temp = BMP280_OS_4X;
    /* Pressure over sampling none (disabling pressure measurement) */
    conf.os_pres = BMP280_OS_NONE;
    /* Setting the output data rate as 1HZ(1000ms) */
    conf.odr = BMP280_ODR_1000_MS;
    rslt = bmp280_set_config(&conf, &bmp);
    /* Always set the power mode after setting the configuration */
    rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);

	display.init();
    display.clearDisplay();
	// epd_fast:    LovyanGFX uses a 4×4 16pixel tile pattern to display a pseudo 17level grayscale.
	// epd_quality: Uses 16 levels of grayscale
	display.setEpdMode(epd_mode_t::epd_quality);
	//display.setFont(&AsciiFont24x48);
    display.setFont(&Ubuntu_M48pt8b);
    while (1)
    {
        /* Reading the raw data from sensor */
        rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

        /* Getting the 32 bit compensated temperature */
        rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);

        /* Getting the compensated temperature as floating point value */
        rslt = bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);
        printf("T: %f refresh\n", temp);
		display.setCursor(100,EPD_HEIGHT/2);
		display.printf("%.2f C", temp);
        /* Sleep time between measurements = BMP280_ODR_1000_MS */
        bmp.delay_ms(3000);
    }

}

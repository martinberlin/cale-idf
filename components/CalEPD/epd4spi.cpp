/* SPI Master IO class */
#include <epd4spi.h>
#include <string.h>
#include "freertos/task.h"
#include "esp_log.h"

#ifdef CONFIG_IDF_TARGET_ESP32
    #define EPD_HOST    HSPI_HOST
    #define DMA_CHAN    2
#elif defined CONFIG_IDF_TARGET_ESP32S2
    #define EPD_HOST    SPI2_HOST
    #define DMA_CHAN    EPD_HOST
#elif defined CONFIG_IDF_TARGET_ESP32S3
    #define EPD_HOST    SPI2_HOST
    #define DMA_CHAN    SPI_DMA_CH_AUTO
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #define EPD_HOST    SPI3_HOST
    #define DMA_CHAN    SPI_DMA_CH_AUTO
#endif

/** DISPLAYS REF:
__________
| S2 | M2 |
-----------
| M1 | S1 |
-----------
*/

void Epd4Spi::init(uint8_t frequency=4,bool debug=false){
    debug_enabled = debug;
    printf("PIN SETUP:\nSPI_M1_CS:%d <- all set as output GPIOs\nSPI_S1_CS:%d\nSPI_M2_CS:%d\nSPI_S2_CS:%d\n",
    CONFIG_EINK_SPI_M1_CS,CONFIG_EINK_SPI_S1_CS,CONFIG_EINK_SPI_M2_CS,CONFIG_EINK_SPI_S2_CS);
    // Initialize GPIOs direction & initial states. Check that all are set
    // Setting 16 as output resets the ESP32 in TinyPICO (Don't use tinyPICO for this)
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_M1_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_S1_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_M2_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_S2_CS, GPIO_MODE_OUTPUT);

    printf("\nSPI_M1_BUSY:%d <- all set as input,pullup GPIOs\nSPI_S1_BUSY:%d\nSPI_M2_BUSY:%d\nSPI_S2_BUSY:%d\n",
    CONFIG_EINK_SPI_M1_BUSY,CONFIG_EINK_SPI_S1_BUSY,CONFIG_EINK_SPI_M2_BUSY,CONFIG_EINK_SPI_S2_BUSY);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_SPI_M1_BUSY, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_SPI_S1_BUSY, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_SPI_M2_BUSY, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_SPI_S2_BUSY, GPIO_PULLUP_ONLY);

    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_M1_BUSY, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_S1_BUSY, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_M2_BUSY, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_S2_BUSY, GPIO_MODE_INPUT);

    printf("\nM1S1_DC:%d <- all set as output GPIOs\nM2S2_DC:%d\nM1S1_RST:%d\nM2S2_RST:%d\n",
    CONFIG_EINK_M1S1_DC,CONFIG_EINK_M2S2_DC,CONFIG_EINK_M1S1_RST,CONFIG_EINK_M2S2_RST);
    
    gpio_set_direction((gpio_num_t)CONFIG_EINK_M1S1_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_M2S2_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_M1S1_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_M2S2_RST, GPIO_MODE_OUTPUT);

    // Chip select starts HIGH since only on LOW transmits
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CLK, 0);
    
    esp_err_t ret;
    // MISO not used, only Master to Slave
    spi_bus_config_t buscfg={
        .mosi_io_num=CONFIG_EINK_SPI_MOSI,
        .miso_io_num = -1,
        .sclk_io_num=CONFIG_EINK_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4094
    };
    // max_transfer_sz   4Kb is the defaut SPI transfer size if 0
    // debug: 50000  0.5 Mhz so we can sniff the SPI commands with a Slave
    uint16_t multiplier = 1000;
    if (debug_enabled) {
        frequency = 50;
        multiplier = 1;
    }
    // Config Frequency and SS GPIO
    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*multiplier*1000,  // DEBUG: 50000 - No debug usually 4 Mhz
        .input_delay_ns=0,
        .flags = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
        .queue_size=5
    };
    // Note: .spics_io_num=-1 is disabled since there are 4 Chip selects

    ret=spi_bus_initialize(EPD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // Attach the EPD to the SPI bus
    ret=spi_bus_add_device(EPD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    
    if (debug_enabled) {
      printf("EpdSpi::init() Debug enabled. SPI master at frequency:%d  MOSI:%d CLK:%d\n",
      frequency*multiplier*1000, CONFIG_EINK_SPI_MOSI, CONFIG_EINK_SPI_CLK);
        }
    }

/* This ones will redirect it to M1 */
void Epd4Spi::cmd(const uint8_t cmd) {
    cmdM1(cmd);
}
void Epd4Spi::data(const uint8_t data) {
    dataM1(data);
}

/* M1 */
void Epd4Spi::cmdM1(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("M1C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 1); 
}

void Epd4Spi::dataM1(uint8_t data)
{
    if (debug_enabled) {
      printf("D %x\n",data);
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;              //The data is the cmd itself
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 1);
}

/* S1 */
void Epd4Spi::cmdS1(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("S1C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 1);
}

void Epd4Spi::dataS1(uint8_t data)
{
    if (debug_enabled) {
      printf("D %x\n",data);
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;              //The data is the cmd itself
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
}

/* M2 */
void Epd4Spi::cmdM2(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("M2C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 1);
}

void Epd4Spi::dataM2(uint8_t data)
{
    if (debug_enabled) {
      printf("D %x\n",data);
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
}

/* S2 */
void Epd4Spi::cmdS2(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("S2C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 1);
}

void Epd4Spi::dataS2(uint8_t data)
{
    if (debug_enabled) {
      printf("D %x\n",data);
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
}

// Send a command to all 4 displays
void Epd4Spi::cmdM1S1M2S2(uint8_t cmd) {
    if (debug_enabled) {
        printf("All4 C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_DC, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_DC, 1);
}

// Send data to the 4 displays
void Epd4Spi::dataM1S1M2S2(uint8_t data)
{
    /* if (debug_enabled) {
      printf("D %x\n",data);
    } */
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
}

void Epd4Spi::data(const uint8_t *data, int len)
{
    dataM1(data, len);
}

/* Send data to the SPI. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void Epd4Spi::dataM1(const uint8_t *data, int len)
{
    if (len==0) return;
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M1_CS, 1);
}
void Epd4Spi::dataM2(const uint8_t *data, int len)
{
    if (len==0) return;
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_M2_CS, 1);
}
void Epd4Spi::dataS1(const uint8_t *data, int len)
{
    if (len==0) return;
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S1_CS, 1);
}
void Epd4Spi::dataS2(const uint8_t *data, int len)
{
    if (len==0) return;
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    ret=spi_device_polling_transmit(spi, &t);
    assert(ret==ESP_OK);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_S2_CS, 1);
}

void Epd4Spi::reset(uint8_t millis=20) {
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_RST, 0);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_RST, 0);
    vTaskDelay(millis / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M1S1_RST, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_M2S2_RST, 1);
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

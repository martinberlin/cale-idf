/* SPI Master IO class */
#include <epdspi2cs.h>
#include <string.h>
#include "freertos/task.h"
#include "esp_log.h"
#ifdef CONFIG_IDF_TARGET_ESP32
#define EPD_HOST    HSPI_HOST
#define DMA_CHAN    2

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define EPD_HOST    SPI2_HOST
#define DMA_CHAN    EPD_HOST
#endif

void EpdSpi2Cs::init(uint8_t frequency=4,bool debug=false){
    debug_enabled = debug;
    //Initialize GPIOs direction & initial states
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS2, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_MISO, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_BUSY, GPIO_PULLUP_ONLY);

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    
    esp_err_t ret;
    // Here MISO is used, to receive temp & accelerometer data
    spi_bus_config_t buscfg={
        .mosi_io_num=CONFIG_EINK_SPI_MOSI,
        .miso_io_num=CONFIG_EINK_SPI_MISO,
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
    //Config Frequency and SS GPIO
    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*multiplier*1000,  // DEBUG: 50000 - No debug usually 4 Mhz
        .input_delay_ns=0,
        .spics_io_num=CONFIG_EINK_SPI_CS,
        .flags=SPI_DEVICE_HALFDUPLEX,
        .queue_size=5
    };
    // DISABLED Callbacks pre_cb/post_cb. SPI does not seem to behave the same
    // CS / DC GPIO states the usual way

    //Initialize the SPI bus
    ret=spi_bus_initialize(EPD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    //Attach the EPD to the SPI bus
    ret=spi_bus_add_device(EPD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    // debug_enabled
    if (true) {
      printf("EpdSpi::init() Debug enabled. SPI master at frequency:%d  MOSI:%d CLK:%d CS:%d DC:%d RST:%d BUSY:%d\n",
      frequency*multiplier*1000, CONFIG_EINK_SPI_MOSI, CONFIG_EINK_SPI_CLK, CONFIG_EINK_SPI_CS,
      CONFIG_EINK_DC,CONFIG_EINK_RST,CONFIG_EINK_BUSY);
        }
    }

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void EpdSpi2Cs::cmd(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("C %x\n",cmd);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself 

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
}

void * EpdSpi2Cs::readTemp()
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
    data(EPD_REGREAD | 0x08);

    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);

    if (debug_enabled) {
        printf("B0 %d 1: %d 2: %d 3: %d\n", t.rx_data[0], t.rx_data[1], t.rx_data[2], t.rx_data[3]);
    }
    return t.rx_buffer;
}

uint8_t EpdSpi2Cs::readRegister(uint8_t address) {
        esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
    data(address | EPD_REGREAD);

    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);

    if (debug_enabled) {
        printf("B0 %d 1: %d 2: %d 3: %d\n", t.rx_data[0], t.rx_data[1], t.rx_data[2], t.rx_data[3]);
    }
    return 1;
}


/**
 * Data does not toogle CS
 */
void EpdSpi2Cs::data(const uint8_t data)
{
    if (debug_enabled) {
        printf("D %x\n",data);
    }

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;              //The data is the cmd itself 
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
}
// Non used
void EpdSpi2Cs::data(const uint8_t *data, int len) {}

void EpdSpi2Cs::csStateLow() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
}
void EpdSpi2Cs::csStateHigh() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
}
void EpdSpi2Cs::csStateToogle() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
}
void EpdSpi2Cs::cs2StateLow() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 0);
}
void EpdSpi2Cs::cs2StateHigh() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 1);
}
void EpdSpi2Cs::cs2StateToogle() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 0);
}

/**
 * Use CS2 pin for this communication
 */
void EpdSpi2Cs::cmdAccel(const uint8_t *data, int len)
{
    if (len==0) return; 
    if (debug_enabled) {
        printf("D\n");
        for (int i = 0; i < len; i++)  {
            printf("%x ",data[i]);
        }
        printf("\n");
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 0);
    esp_err_t ret;
    spi_transaction_t t;
                
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 1);
}

void EpdSpi2Cs::reset(uint8_t millis=5) {
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    vTaskDelay(millis / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
    vTaskDelay(millis / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    vTaskDelay(millis / portTICK_RATE_MS);
}
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
#elif defined CONFIG_IDF_TARGET_ESP32S3
    #define EPD_HOST    SPI2_HOST
    #define DMA_CHAN    SPI_DMA_CH_AUTO
#elif defined CONFIG_IDF_TARGET_ESP32C3
    // chip only support spi dma channel auto-alloc
    #define EPD_HOST    SPI2_HOST
    #define DMA_CHAN    SPI_DMA_CH_AUTO
#endif


void EpdSpi2Cs::init(uint8_t frequency=4,bool debug=false){
    debug_enabled = debug;
        // debug_enabled
        // debug: 50000  0.5 Mhz so we can sniff the SPI commands with a Slave
    uint16_t multiplier = 1000;
    if (true) {
      printf("EpdSpi::init() Debug enabled. SPI master at frequency:%d  MOSI:%d MISO: %d CLK:%d CS:%d RST:%d BUSY:%d\n",
      frequency*multiplier*1000, CONFIG_EINK_SPI_MOSI, CONFIG_EINK_SPI_MISO, CONFIG_EINK_SPI_CLK, CONFIG_EINK_SPI_CS,
      CONFIG_EINK_RST,CONFIG_EINK_BUSY);
        }
    //Initialize GPIOs direction & initial states. MOSI/MISO are setup by SPI interface
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS2, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_BUSY, GPIO_PULLUP_ONLY);

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS2, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    
    esp_err_t ret;
    // Here MISO is used, to receive temp & accelerometer data
    // BUFFER max transfer is 21" buffer *2
    spi_bus_config_t buscfg={
        .mosi_io_num=CONFIG_EINK_SPI_MOSI,
        .miso_io_num=CONFIG_EINK_SPI_MISO,
        .sclk_io_num=CONFIG_EINK_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=32768
    };
    // max_transfer_sz   4Kb is the defaut SPI transfer size if 0
    
    if (debug_enabled) {
        frequency = 50;
        multiplier = 1;
    }
    //Config Frequency and SS GPIO. Full duplex SPI:
    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*multiplier*1000,  // DEBUG: 50000 - No debug usually 4 Mhz
        .spics_io_num=CONFIG_EINK_SPI_CS,
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

    }

// Release SPI connection
void EpdSpi2Cs::release() {
    printf("Releasing SPI bus\n");
    esp_err_t ret;
    ret = spi_bus_remove_device(spi);
    ESP_ERROR_CHECK(ret);
    printf("Free heap: %d after releasing SPI\n", (int)xPortGetFreeHeapSize());
}

/* Send a command to the Epaper. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete. 
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 * 
 * Note: Adding spics_io_num to spi_device_interface_config_t toggles Chip Select
 * GPIO automatically low when the transaction starts and high at the end.
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
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);            //Should have had no issues.
}

uint8_t EpdSpi2Cs::readTemp()
{
    uint8_t regTemp[2] = {EPD_REGREAD | 0x08, 0xFF};
    return readRegister(regTemp, sizeof(regTemp));
}

uint8_t EpdSpi2Cs::readRegister(const uint8_t *data, int len)
{
    if (len==0) return 0;
    if (debug_enabled) {
        printf("READ: \n");
        for (int i = 0; i < len; i++)  {
            printf("%x ",data[i]);
        }
        printf("\n");
    }

    esp_err_t ret;
    spi_transaction_t t;
                
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    t.flags = SPI_TRANS_USE_RXDATA;
    // There is no need to toogle CS when is defined in spi_config struct: spics_io_num
    //gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);

    waitForBusy();
    uint16_t response = *(uint16_t*) t.rx_data;
    uint8_t readByte = (response >> (8*1)) & 0xff;
    return readByte;
}

void EpdSpi2Cs::waitForBusy()
{
    int64_t time_since_boot = esp_timer_get_time();

    while (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0){
        vTaskDelay(10/portTICK_PERIOD_MS); 

        if (esp_timer_get_time()-time_since_boot>500000)
        {
        break;
        }
    }
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

/**
 * Send multiple data in one transaction
 */
void EpdSpi2Cs::data(const uint8_t *data, int len)
{
    if (len==0) return;
    if (debug_enabled) {
        printf("D\n");
        for (int i = 0; i < len; i++)  {
            printf("%x ",data[i]);
        }
        printf("\n");
    }
    esp_err_t ret;
    spi_transaction_t t;
                
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    ret=spi_device_polling_transmit(spi, &t);

    assert(ret==ESP_OK);
}

void EpdSpi2Cs::reset(uint8_t millis=5) {
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
    vTaskDelay(millis / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

/* SPI Master IO class */
#include <epdspi.h>
#include <string.h>
#include "freertos/task.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST    SPI2_HOST
#define DMA_CHAN    LCD_HOST
#endif


//This function is called (in irq context!) just before a transmission starts. 
//It will set the D/C line to the value indicated in the user field.
void spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, dc);
}

void EpdSpi::init(uint8_t frequency=4,bool debug=false){
    debug_enabled = debug;
    //Initialize GPIOs direction & initial states
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_BUSY, GPIO_PULLUP_ONLY);

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    
    esp_err_t ret;
    // MISO not used, only Master to Slave
    spi_bus_config_t buscfg={
        .mosi_io_num=CONFIG_EINK_SPI_MOSI,
        .miso_io_num = -1,
        .sclk_io_num=CONFIG_EINK_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=3000*1000/8
    };
    //max_transfer_sz set to the bigger test display

    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*1000*1000,  // As default 4 MHz
        .input_delay_ns = 0,
        .spics_io_num=CONFIG_EINK_SPI_CS,
        .flags = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
        .queue_size=10,                        //We want to be able to queue 7 transactions at a time
        .pre_cb=spi_pre_transfer_callback,     //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    //Attach the EPD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    if (debug_enabled) {
      printf("EpdSpi::init() SPI initialized at %d Mhz. MOSI:%d CLK:%d CS:%d\n",
      frequency, CONFIG_EINK_SPI_MOSI, CONFIG_EINK_SPI_CLK, CONFIG_EINK_SPI_CS);
        }
    }

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void EpdSpi::cmd(const uint8_t cmd)
{
    if (debug_enabled) {
        printf("cmd(%d)\n",cmd);
    }
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 1);   
}

void EpdSpi::data(uint8_t data)
{
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&data;              //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 1);
}

/* Send data to the SPI. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void EpdSpi::data(const uint8_t *data, int len)
{
  if (len==0) return; 
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
    esp_err_t ret;
    spi_transaction_t t;
                
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
}

void EpdSpi::reset() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
    vTaskDelay(10 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    vTaskDelay(10 / portTICK_RATE_MS);
}

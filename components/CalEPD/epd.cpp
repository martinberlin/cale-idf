/* SPI Master example refactored for epaper */
#include "epd.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST    SPI2_HOST
#define DMA_CHAN    LCD_HOST
#endif

/*
 The EPD needs a bunch of command/argument values to be initialized.
 They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[42];
    uint8_t databytes; //No of data in data; 0xFF = end of cmds.
} epd_init_42;

typedef struct {
    uint8_t cmd;
    uint8_t data[44];
    uint8_t databytes;
} epd_init_44;

typedef struct {
    uint8_t cmd;
    uint8_t data[1];
} epd_init_1;

typedef struct {
    uint8_t cmd;
    uint8_t data[2];
} epd_init_2;

typedef struct {
    uint8_t cmd;
    uint8_t data[3];
} epd_init_3;

typedef struct {
    uint8_t cmd;
    uint8_t data[5];
} epd_power_5;

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//full screen update LUT
DRAM_ATTR static const epd_init_44 lut_20_vcomDC={
0x20, {
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
},44};

DRAM_ATTR static const epd_init_42 lut_21_ww={
0x21, {
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR static const epd_init_42 lut_22_bw={
0x22,{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR static const epd_init_42 lut_23_wb ={
0x23,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR static const epd_init_42 lut_24_bb ={
0x24,{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},42};

DRAM_ATTR static const epd_power_5 epd_wakeup_power ={
0x01,{0x03,0x00,0x2b,0x2b,0x03}
};

DRAM_ATTR static const epd_init_3 epd_soft_start ={
0x06,{0x17,0x17,0x17}
};

DRAM_ATTR static const epd_init_2 epd_panel_setting ={
0x00,{0xbf,0x0d}
};

DRAM_ATTR static const epd_init_1 epd_pll ={
0x30,{0x3a}
};

DRAM_ATTR static const epd_init_3 epd_resolution ={
0x61,{GxGDEW0213I5F_WIDTH,
GxGDEW0213I5F_HEIGHT >> 8,
GxGDEW0213I5F_HEIGHT & 0xFF
}};

//partial screen update LUT
//#define Tx19 0x19 // original value is 25 (phase length)
#define Tx19 0x28   // new value for test is 40 (phase length)
/* const unsigned char Epd::lut_20_vcomDC_partial[] =
{
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
};

const unsigned char Epd::lut_21_ww_partial[] =
{
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char Epd::lut_22_bw_partial[] =
{
  0x80, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char Epd::lut_23_wb_partial[] =
{
  0x40, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char Epd::lut_24_bb_partial[] =
{
  0x00, Tx19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
}; */

// Partial Update Delay, may have an influence on degradation
#define GxGDEW0213I5F_PU_DELAY 100

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
//DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
 
//Constructor
Epd::Epd():Adafruit_GFX(GxGDEW0213I5F_WIDTH, GxGDEW0213I5F_HEIGHT){
  
  printf("Epd() constructor extends Adafruit_GFX(%d,%d)\n",
  GxGDEW0213I5F_WIDTH, GxGDEW0213I5F_HEIGHT);  
  }
/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void Epd::cmd(const uint8_t cmd)
{
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 0);
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.

    //vTaskDelay(10 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 1);
    
}

void Epd::data(uint8_t data)
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
void Epd::data(const uint8_t *data, int len)
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

//This function is called (in irq context!) just before a transmission starts. 
//It will set the D/C line to the value indicated in the user field.
void spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, dc);
}

void Epd::spi_reset() {
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
    vTaskDelay(20 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
    vTaskDelay(20 / portTICK_RATE_MS);
}

void Epd::initFullUpdate(){
    cmd(0x82);  //vcom_DC setting
    data(0x08);

    cmd(0X50); //VCOM AND DATA INTERVAL SETTING
    data(0x97);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57

    cmd(lut_20_vcomDC.cmd);
    data(lut_20_vcomDC.data,lut_20_vcomDC.databytes);
   
    cmd(lut_21_ww.cmd);
    data(lut_21_ww.data,lut_21_ww.databytes);

    cmd(lut_22_bw.cmd);
    data(lut_22_bw.data,lut_22_bw.databytes);

    cmd(lut_23_wb.cmd);
    data(lut_23_wb.data,lut_23_wb.databytes);

    cmd(lut_24_bb.cmd);
    data(lut_24_bb.data,lut_24_bb.databytes);
    printf("initFullUpdate() LUT\n");
}

//Initialize the display
void Epd::spi_init()
{
    printf("spi_init()\n");
    //Initialize non-SPI GPIOs
    gpio_set_direction((gpio_num_t)CONFIG_EINK_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CONFIG_EINK_BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EINK_BUSY, GPIO_PULLUP_ONLY);

    gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 1);
    gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);

    //Reset the display
    spi_reset();
    fillScreen(GxEPD_WHITE);
    _using_partial_mode = false;
}

void Epd::fillScreen(uint16_t color)
{
  uint8_t data = (color == GxEPD_WHITE) ? 0xFF : 0x00;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  }

  printf("fillScreen(%d) _buffer len:%d\n",data,sizeof(_buffer));
}

void Epd::_wakeUp(){
    //if (_rst >= 0) {
      spi_reset();
    //}

   
    cmd(epd_wakeup_power.cmd);
    data(epd_wakeup_power.data,5);
   
    cmd(epd_soft_start.cmd);
    data(epd_soft_start.data,3);
    cmd(0x04);
   _waitBusy("epd_wakeup_power");

  // [1] LUT from register, 128x296
  // [2] VCOM to 0V fast
    cmd(epd_panel_setting.cmd);
    data(epd_panel_setting.data,2);

  // 3a 100HZ   29 150Hz 39 200HZ 31 171HZ
   cmd(epd_pll.cmd);
   data(epd_pll.data,1);   

  //resolution setting
    cmd(epd_resolution.cmd);
    data(epd_resolution.data,3);

   initFullUpdate();
}

void Epd::update()
{
  _using_partial_mode = false;
  _wakeUp();

  cmd(0x10);
  // In GxEPD here it wrote the full buffer with 0xFF
  cmd(0x13);

  data(_buffer,sizeof(_buffer));

  cmd(0x12);
  _waitBusy("display refresh");
  _sleep();
}

void Epd::init(bool debug)
{
    //Initialize the Epaper and reset it
    spi_init();
    debug_enabled = debug;
     if (debug_enabled) {
        printf("EPD SPI initialized. MOSI:%d CLK:%d CS:%d\n",
        CONFIG_EINK_SPI_MOSI, CONFIG_EINK_SPI_CLK, CONFIG_EINK_SPI_CS);
    } 
    esp_err_t ret;
    // MISO not used, only Master to Slave
    spi_bus_config_t buscfg={
        .mosi_io_num=CONFIG_EINK_SPI_MOSI,
        .miso_io_num = -1,
        .sclk_io_num=CONFIG_EINK_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=GxGDEW0213I5F_WIDTH*GxGDEW0213I5F_HEIGHT/8
    };

    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=4*1000*1000,           //Clock out at 4 MHz
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
}

void Epd::_waitBusy(const char* message){
  ESP_LOGI(TAG, "_waitBusy for %s", message);
  int64_t time_since_boot = esp_timer_get_time();

    while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>1000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void Epd::_sleep(){
  cmd(0x02); // power off display
  _waitBusy("power_off");
  cmd(0x07); // deep sleep
  data(0xa5);
}

void Epd::drawPixel(int16_t x, int16_t y, uint16_t color) {
  //printf("Epd drawPixel(%d,%d)\n",x,y);
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = GxGDEW0213I5F_WIDTH - x - 1;
      break;
    case 2:
      x = GxGDEW0213I5F_WIDTH - x - 1;
      y = GxGDEW0213I5F_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = GxGDEW0213I5F_HEIGHT - y - 1;
      break;
  }
  uint16_t i = x / 8 + y * GxGDEW0213I5F_WIDTH / 8;
  // Todo research what is for _current_page
  if (_current_page < 1)
  {
    if (i >= sizeof(_buffer)) return;
  }
  else
  {
    y -= _current_page * GxGDEW0213I5F_PAGE_HEIGHT;
    if ((y < 0) || (y >= GxGDEW0213I5F_PAGE_HEIGHT)) return;
    i = x / 8 + y * GxGDEW0213I5F_WIDTH / 8;
  }

  if (color)
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
  else
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

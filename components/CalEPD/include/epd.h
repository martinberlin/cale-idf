#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <math.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include <string.h>

#include <Adafruit_GFX.h>
// the only colors supported by any of these displays; mapping of other colors is class specific
#define GxEPD_BLACK     0x0000
#define GxEPD_DARKGREY  0x7BEF      /* 128, 128, 128 */
#define GxEPD_LIGHTGREY 0xC618      /* 192, 192, 192 */
#define GxEPD_WHITE     0xFFFF
#define GxEPD_RED       0xF800      /* 255,   0,   0 */

#define GxGDEW0213I5F_WIDTH 104
#define GxGDEW0213I5F_HEIGHT 212
#define GxGDEW0213I5F_BUFFER_SIZE (uint32_t(GxGDEW0213I5F_WIDTH) * uint32_t(GxGDEW0213I5F_HEIGHT) / 8)
// divisor for AVR, should be factor of GxGDEW0213I5F_HEIGHT
#define GxGDEW0213I5F_PAGES 4
#define GxGDEW0213I5F_PAGE_HEIGHT (GxGDEW0213I5F_HEIGHT / GxGDEW0213I5F_PAGES)
#define GxGDEW0213I5F_PAGE_SIZE (GxGDEW0213I5F_BUFFER_SIZE / GxGDEW0213I5F_PAGES)

// Note: GxGDEW0213I5F is our test display that will be the default initializing this class
class Epd : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "Epd driver";
    spi_device_handle_t spi;
    Epd();

    void cmd(const uint8_t cmd);
    void data(uint8_t data);
    void data(const uint8_t *data, int len);
      
    void drawPixel(int16_t x, int16_t y, uint16_t color);  

    void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata);

    void send_line_finish(spi_device_handle_t spi);

    void display_pretty_colors(spi_device_handle_t spi);
    
    // EPD tests 
    void init(bool debug);
    void initFullUpdate();
    void spi_reset();
    void spi_init();
    void fillScreen(uint16_t color);
    void update();

    void _wakeUp();
    void _sleep();
    void _waitBusy();

  private:
    uint8_t _buffer[GxGDEW0213I5F_BUFFER_SIZE];
    int16_t _current_page = 1;
    bool _using_partial_mode;
    bool debug_enabled;
    // Probably this LUT commands should be private members of Epd 
    //const unsigned char lut_20_vcomDC_partial[];
};
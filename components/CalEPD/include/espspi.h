#include "driver/spi_master.h"
#include "driver/gpio.h"

class EspSpi
{
  public:
    spi_device_handle_t spi;
    
    EspSpi();
    void cmd(const uint8_t cmd);
    void data(uint8_t data);
    void data(const uint8_t *data, int len);
    void reset();
    void init();
};
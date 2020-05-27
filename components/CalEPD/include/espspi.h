/* Implement IoInterface for SPI communication */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

class EspSpi : IoInterface
{
  public:
    spi_device_handle_t spi;
    
    EspSpi();
    void cmd(const uint8_t cmd);
    void data(uint8_t data);
    void data(const uint8_t *data, int len);
    void reset();
    void init(uint8_t frequency);
};
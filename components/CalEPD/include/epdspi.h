/* Implement IoInterface for SPI communication */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#ifndef epdspi_h
#define epdspi_h
class EpdSpi : IoInterface
{
  public:
    spi_device_handle_t spi;

    void cmd(const uint8_t cmd) override;
    void data(uint8_t data) override;
    void dataBuffer(uint8_t data);
    void data(const uint8_t *data, int len) override;
    
    void reset(uint8_t millis) override;
    void init(uint8_t frequency, bool debug) override;
  private:
    bool debug_enabled = true;
};
#endif
// Note: using override compiler will issue an error for "changing the type"
//       in case the type is changed.
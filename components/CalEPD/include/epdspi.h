/* Implement IoInterface for SPI communication */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#ifndef epdspi_h
#define epdspi_h
// : IoInterface
class EpdSpi 
{
  public:
    spi_device_handle_t spi;

    void cmd(const uint8_t cmd) ; // Should override if IoInterface is there
    void data(uint8_t data) ;
    void dataBuffer(uint8_t data);
    void data(const uint8_t *data, int len) ;
    
    void reset(uint8_t millis) ;
    void init(uint8_t frequency, bool debug) ;
  private:
    bool debug_enabled = true;
};
#endif
// Note: using override compiler will issue an error for "changing the type"
//       in case the type is changed.
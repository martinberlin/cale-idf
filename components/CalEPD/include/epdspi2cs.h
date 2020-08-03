/* Implement IoInterface for 4 wire SPI communication with 2 CS pins */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#ifndef epdspi2cs_h
#define epdspi2cs_h
// Instruction R/W bit set HIGH for data READ
#define EPD_REGREAD           0x80

class EpdSpi2Cs : IoInterface
{
  public:
    spi_device_handle_t spi;

    void cmd(const uint8_t cmd) override;
    void data(uint8_t data) override;
    // No data is used here (No DC pin)
    void data(const uint8_t *data, int len) override; 
    void reset(uint8_t millis) override;
    void init(uint8_t frequency, bool debug) override;

    uint8_t readTemp();
    uint8_t readRegister(const uint8_t *data, int len);
    
    // Accelerometer BMA250E uses CS2. Update being done in plastic/accelerometer branch
    void waitForBusy();
  private:
    bool debug_enabled = true;
};
#endif
// Note: using override compiler will issue an error for "changing the type"
//       in case the type is changed.
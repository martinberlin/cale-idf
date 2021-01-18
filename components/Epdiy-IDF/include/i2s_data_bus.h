/**
 * Implements a 8bit parallel interface to transmit pixel
 * data to the display, based on the I2S peripheral.
 */

#pragma once

#include "driver/gpio.h"
#include "esp_attr.h"
#include <stdint.h>
// i2s_data_bus.cpp
#include "driver/periph_ctrl.h"
#include "esp32/rom/lldesc.h"
#include "esp_heap_caps.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/rtc.h"

/**
 * I2S bus configuration parameters.
 */
typedef struct {
  /// GPIO numbers of the parallel bus pins.
  gpio_num_t data_0;
  gpio_num_t data_1;
  gpio_num_t data_2;
  gpio_num_t data_3;
  gpio_num_t data_4;
  gpio_num_t data_5;
  gpio_num_t data_6;
  gpio_num_t data_7;

  /// Data clock pin.
  gpio_num_t clock;

  /// "Start Pulse", enabling data input on the slave device (active low)
  gpio_num_t start_pulse;

  // Width of a display row in pixels.
  uint32_t epd_row_width;
} i2s_bus_config;


class I2SDataBus
{
  public:
    // Indicates the device has finished its transmission and is ready again.
    static volatile bool output_done;

    //static intr_handle_t gI2S_intr_handle;
    /**
     * Initialize the I2S data bus for communication
     * with a 8bit parallel display interface.
     */
    void i2s_bus_init(i2s_bus_config *cfg);

    // Set up a GPIO as output and route it to a signal.
    void gpio_setup_out(int gpio, int sig, bool invert);

    // Initializes a DMA descriptor.
    void fill_dma_desc(volatile lldesc_t *dmadesc, uint8_t *buf,
                          i2s_bus_config *cfg);
    // Address of the currently front DMA descriptor
    uint32_t dma_desc_addr();

    /**
     * Resets "Start Pulse" signal when the current row output is done
     */
    //void IRAM_ATTR i2s_int_hdl(void *arg);

    /**
     * Get the currently writable line buffer.
     */
    //uint8_t IRAM_ATTR *i2s_get_current_buffer();
    volatile uint8_t* IRAM_ATTR i2s_get_current_buffer();

    /**
     * Switches front and back line buffer.
     * If the switched-to line buffer is currently in use,
     * this function blocks until transmission is done.
     */
    void IRAM_ATTR i2s_switch_buffer();

    /**
     * Start transmission of the current back buffer.
     */
    void IRAM_ATTR i2s_start_line_output();

    /**
     * Returns true if there is an ongoing transmission.
     */
    bool IRAM_ATTR i2s_is_busy();

    /**
     * Give up allocated resources.
     */
    void i2s_deinit();

  private:

    // Internal debugging
    bool debug_enabled = true;
    // The start pulse pin extracted from the configuration for use in the "done"
    gpio_num_t start_pulse_pin;
    /// DMA descriptors for front and back line buffer.

    // We use two buffers, so one can be filled while the other is transmitted.
    typedef struct {
      volatile lldesc_t *dma_desc_a;
      volatile lldesc_t *dma_desc_b;

      /// Front and back line buffer.
      uint8_t *buf_a;
      uint8_t *buf_b;
    } i2s_parallel_state_t;

    /// Indicates which line buffer is currently back / front.
    int current_buffer = 0;

    /// The I2S state instance.
    i2s_parallel_state_t i2s_state;

    //static void IRAM_ATTR i2s_int_hdl(void *arg);
    
};
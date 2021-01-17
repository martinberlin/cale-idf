Epdiy component is only for parallel epapers driven by 8 GPIO datalines
plus latch PIN and CLK. It's not for SPI (Use CalEPD for SPI epapers)

The idea is to refactor original class of Epdiy:

https://github.com/vroland/epdiy

As a C++ class to inject into this parallel data-bus displays. First target to drive is the 4.7 inch epaper from LILYGO
This is by no means ready and is only a raw starts. Please avoid using this code or parts of it in any
productive environment.
First epaper target is LILYGO:

ED047TC1 	4.7" 	960 x 540 pixels

As a blueprint I can imagine this is going to happen in the same way as SPI right now. As an example:

- - - - - - - - - - - - - - - - 
// Include header
#include <ED047TC1.h>

// Instantiate the parallel I2S Input/Output class that in turn has it's own Kconfig for the 8 GPIOs + clock
I2SDataBus io;
ED047TC1 display(io);

// Do stuff with the epaper
display.println("Hello world");
display.update();

- - - - - - - - - - - - - - - - 

This directory is empty since the class is still in the conception phase and holds only the beginning of the Kconfig PIN configurations.

Program flow (Instantiation from Epdiy epaper TTGO 4.7)

app_main - STARTPOINT

  heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
  heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
  epd_init(); ->
    epd_driver.c epd_base_init(EPD_WIDTH);
    CALLS epd_base_init(uint32_t epd_row_width)

        static void config_reg_init(epd_config_register_t *cfg) {
        cfg->ep_latch_enable = false;
        cfg->power_disable = true;
        cfg->pos_power_enable = false;
        cfg->neg_power_enable = false;
        cfg->ep_stv = true;
        cfg->ep_scan_direction = true;
        cfg->ep_mode = false;
        cfg->ep_output_enable = false;
        }
    // Configures GPIO directions for Config lines (Power Control Output/Off)
        gpio_set_direction(CFG_DATA, GPIO_MODE_OUTPUT);
        gpio_set_direction(CFG_CLK, GPIO_MODE_OUTPUT);
        gpio_set_direction(CFG_STR, GPIO_MODE_OUTPUT);
    // Sends config registers
        push_cfg(&config_reg);
    // Setups I2S
      // add an offset off dummy bytes to allow for enough timing headroom
        i2s_config.epd_row_width = epd_row_width + 32;
        i2s_config.clock = CKH;
        i2s_config.start_pulse = STH;
        i2s_config.data_0 = D0;
        i2s_config.data_1 = D1;
        i2s_config.data_2 = D2;
        i2s_config.data_3 = D3;
        i2s_config.data_4 = D4;
        i2s_config.data_5 = D5;
        i2s_config.data_6 = D6;
        i2s_config.data_7 = D7;

    // Creates two Semaphores (DMA)
    feed_params.done_smphr = xSemaphoreCreateBinary();
    feed_params.start_smphr = xSemaphoreCreateBinary();
    // Instantiate Output queue
    output_queue = xQueueCreate(32, EPD_WIDTH / 2);

  // Creates the pixelBuffer and fills it with 0xFF
  framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  // At this point the epaper seems configured and ready to be powered on
  epd_poweron(); -> ed097oc4.c (model)
    cfg_poweron(&config_reg); -> calls config_reg_v2.h for LILYGO
    // Sends initial configuration (That is somehow different for LILYGO setting cfg->ep_scan_direction = true)
    CALLS push_cfg(const epd_config_register_t *cfg) 
    // push config bits in reverse order via CFG_* lines
    push_cfg_bit(*every cfg struct);

What push_cfg_bit does:

push_cfg_bit(bool bit) {
  fast_gpio_set_lo(CFG_CLK);
  if (bit) {
    fast_gpio_set_hi(CFG_DATA);
  } else {
    fast_gpio_set_lo(CFG_DATA);
  }
  fast_gpio_set_hi(CFG_CLK);
}

// This config are BITS defined in: 
typedef struct {
  bool ep_latch_enable : 1;
  bool power_disable : 1;
  bool pos_power_enable : 1;
  bool neg_power_enable : 1;
  bool ep_stv : 1;
  bool ep_scan_direction : 1;
  bool ep_mode : 1;
  bool ep_output_enable : 1;
} epd_config_register_t;


ADDITIONAL Functions for EPD control

-> epd_clear_area(Rect_t area) {
  epd_clear_area_cycles(area, 3, clear_cycle_time);
}
-> calls (And this clears the area defined in struct Rect_t)
void epd_clear_area_cycles(Rect_t area, int cycles, int cycle_time) {
  const short white_time = cycle_time;
  const short dark_time = cycle_time;

  for (int c = 0; c < cycles; c++) {
    for (int i = 0; i < 10; i++) {
      epd_push_pixels(area, dark_time, 0);
    }
    for (int i = 0; i < 10; i++) {
      epd_push_pixels(area, white_time, 1);
    }
  }
}

Clear full screen

Rect_t epd_full_screen() {
  Rect_t area = {.x = 0, .y = 0, .width = EPD_WIDTH, .height = EPD_HEIGHT};
  return area;
}

void epd_clear() { epd_clear_area(epd_full_screen()); }

Declaring area and drawing image of 400*300 pixels

  Rect_t board_area = {
    .x = 50,
    .y = 150,
    .width = 400,
    .height = 300,
  };
// Ex. img_board_data defined in giraffe.h as an array
epd_draw_grayscale_image(board_area, (uint8_t*)img_board_data);

calls: epd_draw_image_lines(area, data, mode, NULL);

That sends the array for each line of the image via I2S to the the display

HOW is the epaper feed with this

There are 2 loops running all the time called from epd_init()


  RTOS_ERROR_CHECK(xTaskCreatePinnedToCore((void (*)(void *))provide_out,
                                           "epd_out", 1 << 12, &fetch_params, 5,
                                           NULL, 0));

  RTOS_ERROR_CHECK(xTaskCreatePinnedToCore((void (*)(void *))feed_display,
                                           "epd_render", 1 << 12, &feed_params,
                                           5, NULL, 1));

This loops are triggered by their respective DMA Semaphores only when there is data to send.


PIN configuration
This GPIO config is for the first epaper added that is LILYGO 4.7 

#
# I2S data lines and clock. Defaults to LILYGO epaper ED047TC1
#
CONFIG_D0=33
CONFIG_D1=32
CONFIG_D2=4
CONFIG_D3=19
CONFIG_D4=2
CONFIG_D5=27
CONFIG_D6=21
CONFIG_D7=22
CONFIG_CKV=25
CONFIG_CKH=5
CONFIG_STH=26
CONFIG_CFG_DATA=23
CONFIG_CFG_CLK=18
CONFIG_CFG_STR=0
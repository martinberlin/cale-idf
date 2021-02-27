#include "parallel/ED047TC1touch.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Ed047TC1t::Ed047TC1t(FT6X36& ts): 
  Adafruit_GFX(ED047TC1_WIDTH, ED047TC1_HEIGHT),
  EpdParallel(ED047TC1_WIDTH, ED047TC1_HEIGHT),
  Touch(ts)
{
  printf("Ed047TC1t() w/touch %d*%d\n",
  ED047TC1_WIDTH, ED047TC1_HEIGHT);  

  framebuffer = (uint8_t *)heap_caps_malloc(ED047TC1_WIDTH * ED047TC1_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  memset(framebuffer, 0xFF, ED047TC1_WIDTH * ED047TC1_HEIGHT / 2);
}

//Initialize the display
void Ed047TC1t::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled) printf("Ed047TC1t::init(%d)\n", debug);
    
  epd_init();
  epd_poweron();
  // Initialize touch. Default: 22 FT6X36_DEFAULT_THRESHOLD
  Touch.begin(22, width(), height());
  printf("Touch initialized. Free heap:%d\n",xPortGetFreeHeapSize());

}

void Ed047TC1t::fillScreen(uint16_t color) {
  // Same as: fillRect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color);
  epd_fill_rect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color, framebuffer);
}

void Ed047TC1t::clearScreen()
{
  epd_clear();
}

void Ed047TC1t::clearArea(Rect_t area) {
  epd_clear_area(area);
}

void Ed047TC1t::update(enum DrawMode mode)
{
  epd_draw_image(epd_full_screen(), framebuffer, mode);
}

void Ed047TC1t::powerOn(void)
{
  epd_poweron();
}

void Ed047TC1t::powerOff(){
  epd_poweroff();
}


void Ed047TC1t::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = ED047TC1_WIDTH - x - 1;
      break;
    case 2:
      x = ED047TC1_WIDTH - x - 1;
      y = ED047TC1_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = ED047TC1_HEIGHT - y - 1;
      break;
  }

  epd_draw_pixel(x, y, color, framebuffer);

}

/**
 * Helper method to set both epaper and touch rotation
 */
void Ed047TC1t::displayRotation(uint8_t rotation) {
  if (rotation>3) {
    printf("INVALID rotation value (valid: 0 to 3, got %d) rotation*90\n",rotation);
    return;
  }
  setRotation(rotation);
  Touch.setRotation(rotation);
}

void Ed047TC1t::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
	Touch._touchHandler = fn;
}

void Ed047TC1t::touchLoop(){
  Touch.loop();
}

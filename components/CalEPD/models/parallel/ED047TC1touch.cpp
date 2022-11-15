// Excluded when this define is present
#ifndef CALEPD_EXCLUDE_PARALLEL

#include "parallel/ED047TC1touch.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Ed047TC1t::Ed047TC1t(L58Touch& ts):
  Adafruit_GFX(ED047TC1_WIDTH, ED047TC1_HEIGHT),
  EpdParallel(ED047TC1_WIDTH, ED047TC1_HEIGHT),
  Touch(ts)
{
  printf("Ed047TC1t() w/touch %d*%d\n",
  ED047TC1_WIDTH, ED047TC1_HEIGHT);  
}

//Initialize the display
void Ed047TC1t::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled) printf("Ed047TC1t::init(%d)\n", debug);
    
  epd_init(EPD_OPTIONS_DEFAULT);
  hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
  framebuffer = epd_hl_get_framebuffer(&hl);
  
  epd_poweron();
  // Initialize touch. Default: 22 FT6X36_DEFAULT_THRESHOLD
  Touch.begin(width(), height());
}

void Ed047TC1t::fillScreen(uint16_t color) {
  // Same as old: fillRect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color);
  epd_fill_rect(epd_full_screen(), color, framebuffer);
}

void Ed047TC1t::clearScreen()
{
  epd_clear();
}

void Ed047TC1t::clearArea(EpdRect area) {
  epd_clear_area(area);
}

void Ed047TC1t::update(enum EpdDrawMode mode)
{
  epd_hl_update_screen(&hl, mode, 25);
}


void Ed047TC1t::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, enum EpdDrawMode mode)
{
  if (x >= ED047TC1_WIDTH) {
    printf("Will not update. x position:%d  is major than display max width:%d\n", x, ED047TC1_WIDTH);
    return;
  }
  if (y >= ED047TC1_HEIGHT) {
    printf("Will not update. y position:%d  is major than display max height:%d\n", y, ED047TC1_HEIGHT);
    return;
  }
  _rotate(x, y, w, h);
  
  EpdRect area = {
    .x = x,
    .y = y,
    .width = w,
    .height = h,
  };

  epd_hl_update_area(&hl, mode, 25, area);
  // Not needed. Saved for historical reasons
  /* uint8_t *buffer = (uint8_t *)heap_caps_malloc(w*h/2,MALLOC_CAP_SPIRAM);
  memset(buffer, 0xFF, w*h/2);

  uint32_t i = 0;
  // Crop only this square from the big framebuffer
  for (int16_t y1 = y; y1 < y+h; y1++)
  {
    for (int16_t x1 = x; x1 < x+w; x1=x1+2)
    {
      // 0xf0 fixed -> square with light gray. Issue is when trying to read the pixel
      buffer[i] = framebuffer[y1 *ED047TC1_WIDTH / 2 + x1/2];
      i++;
    }
    //printf("buffer y: %d line: %d\n",y1,i);
  } */
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

void Ed047TC1t::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
    switch (getRotation())
  {
    // 0 landscape
    // 1 90 ° right portrait
    case 1:
      swap(x, y);
      swap(w, h);
      x = height() - x - w;
      break;
    
    case 2:
      // 3 180° landscape
      x = width() - x - w;
      y = height() - y - h;
      break;

    case 3:
      // 3 270 ° portrait
      swap(x, y);
      swap(w, h);
      y = width() - y - h;
      break;
  }
}

void Ed047TC1t::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
	Touch._touchHandler = fn;
}

void Ed047TC1t::registerMultiTouchHandler(void (*fn)(TPoint point1, TPoint point2, TEvent e))
{
	Touch._multiTouchHandler = fn;
}

void Ed047TC1t::touchLoop(){
  Touch.loop();
}

#endif

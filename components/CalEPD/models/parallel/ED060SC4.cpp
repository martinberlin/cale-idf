// Excluded when this define is present
#ifndef CALEPD_EXCLUDE_PARALLEL

#include "parallel/ED060SC4.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Ed060SC4::Ed060SC4(): 
  Adafruit_GFX(ED060SC4_WIDTH, ED060SC4_HEIGHT),
  EpdParallel(ED060SC4_WIDTH, ED060SC4_HEIGHT)
{
  printf("Ed060SC4() %d*%d\n",
  ED060SC4_WIDTH, ED060SC4_HEIGHT);  
}

//Initialize the display
void Ed060SC4::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Ed060SC4::init(%d)\n", debug);
    
    epd_init(EPD_OPTIONS_DEFAULT);
    hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    framebuffer = epd_hl_get_framebuffer(&hl);
    epd_poweron();
}

void Ed060SC4::fillScreen(uint16_t color) {
  // Same as old: fillRect(0, 0, ED060SC4_WIDTH, ED060SC4_HEIGHT, color);
  epd_fill_rect(epd_full_screen(), color, framebuffer);
}

void Ed060SC4::clearScreen()
{
  epd_clear();
}

void Ed060SC4::clearArea(EpdRect area) {
  epd_clear_area(area);
}

void Ed060SC4::update(enum EpdDrawMode mode)
{
  epd_hl_update_screen(&hl, mode, 25);
}

void Ed060SC4::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, enum EpdDrawMode mode, bool using_rotation)
{
  if (x >= ED060SC4_WIDTH) {
    printf("Will not update. x position:%d  is major than display max width:%d\n", x, ED060SC4_WIDTH);
    return;
  }
  if (y >= ED060SC4_HEIGHT) {
    printf("Will not update. y position:%d  is major than display max height:%d\n", y, ED060SC4_HEIGHT);
    return;
  }
  if (using_rotation) _rotate(x, y, w, h);
  
  EpdRect area = {
    .x = x,
    .y = y,
    .width = w,
    .height = h,
  };

  epd_hl_update_area(&hl, mode, 25, area);
}

void Ed060SC4::powerOn(void)
{
  epd_poweron();
}

void Ed060SC4::powerOff(){
  epd_poweroff();
}


void Ed060SC4::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = ED060SC4_WIDTH - x - 1;
      break;
    case 2:
      x = ED060SC4_WIDTH - x - 1;
      y = ED060SC4_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = ED060SC4_HEIGHT - y - 1;
      break;
  }

  epd_draw_pixel(x, y, color, framebuffer);
}

void Ed060SC4::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
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
      // 3 270 ° portrait -> Corrected
      swap(x, y);
      swap(w, h);
      y = width() - y - h;
      break;
  }
  /**
   Enable debug for rotation calculations
      printf("height():%d - y:%d - h:%d\n",width(),y,h);
      printf("x:%d y:%d w:%d h:%d\n",x,y,w,h);
  */
}

#endif

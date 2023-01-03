// Excluded when this define is present
#ifndef CALEPD_EXCLUDE_PARALLEL

#include "parallel/ED047TC1.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
Ed047TC1::Ed047TC1(): 
  Adafruit_GFX(ED047TC1_WIDTH, ED047TC1_HEIGHT),
  EpdParallel(ED047TC1_WIDTH, ED047TC1_HEIGHT)
{
  printf("Ed047TC1() %d*%d\n",
  ED047TC1_WIDTH, ED047TC1_HEIGHT);  

  // Not anymore allocated here. It returns from epd_init_hl
  //framebuffer = (uint8_t *)heap_caps_malloc(ED047TC1_WIDTH * ED047TC1_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  //memset(framebuffer, 0xFF, ED047TC1_WIDTH * ED047TC1_HEIGHT / 2);
}

//Initialize the display
void Ed047TC1::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Ed047TC1::init(%d)\n", debug);
    
    epd_init(EPD_OPTIONS_DEFAULT);
    hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    framebuffer = epd_hl_get_framebuffer(&hl);    
    epd_poweron();
}

void Ed047TC1::fillScreen(uint16_t color) {
  // Same as old: fillRect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color);
  epd_fill_rect(epd_full_screen(), color, framebuffer);
}

void Ed047TC1::clearScreen()
{
  epd_fullclear(&hl, 25);
}

void Ed047TC1::clearArea(EpdRect area) {
  epd_clear_area(area);
}

void Ed047TC1::update(enum EpdDrawMode mode)
{
  epd_hl_update_screen(&hl, mode, 25);
}

void Ed047TC1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, enum EpdDrawMode mode)
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
}

void Ed047TC1::powerOn(void)
{
  epd_poweron();
}

void Ed047TC1::powerOff(){
  epd_poweroff();
}


void Ed047TC1::drawPixel(int16_t x, int16_t y, uint16_t color) {
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

void Ed047TC1::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
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

void Ed047TC1::cpyFramebuffer(uint16_t x, uint16_t y, const void * src, size_t n) {
  memcpy(&framebuffer[y * EPD_WIDTH / 2 + x / 2], src, n);
}

void Ed047TC1::setAllWhite() {
  epd_hl_set_all_white(&hl);
}

#endif

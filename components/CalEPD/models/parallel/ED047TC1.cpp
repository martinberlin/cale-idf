#include "ED047TC1.h"
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
  //framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  //memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

}

void Ed047TC1::drawPixel(int x, int y, uint8_t color, uint8_t *framebuffer) {
  epd_draw_pixel(x, y, color, framebuffer);
}

//Initialize the display
void Ed047TC1::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Ed047TC1::init(%d)\n", debug);
    
    epd_init();
    epd_poweron();
}

//void Ed047TC1::fillScreen(uint16_t color)

void Ed047TC1::clearScreen()
{
  epd_clear();
  return;
  // 0xFF = 8 pixels black, 0x00 = 8 pix. white
  /* uint8_t data = (color == EPD_BLACK) ? ED047TC1_8PIX_BLACK : ED047TC1_8PIX_WHITE;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = data;
  } */

}

void Ed047TC1::clearArea(Rect_t area) {
  epd_clear_area(area);
}

void Ed047TC1::update(uint8_t *framebuffer)
{
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
}

void Ed047TC1::powerOn(void)
{
  epd_poweron();
}

void Ed047TC1::powerOff(){
  epd_poweroff();
}

void Ed047TC1::_rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      swap(w, h);
      x = ED047TC1_WIDTH - x - w - 1;
      break;
    case 2:
      x = ED047TC1_WIDTH - x - w - 1;
      y = ED047TC1_HEIGHT - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = ED047TC1_HEIGHT - y - h - 1;
      break;
  }
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
  uint16_t i = x / 8 + y * ED047TC1_WIDTH / 8;

  // This is the trick to draw colors right. Genious Jean-Marc
  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }
}

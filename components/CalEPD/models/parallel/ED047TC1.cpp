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

  framebuffer = (uint8_t *)heap_caps_malloc(ED047TC1_WIDTH * ED047TC1_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  memset(framebuffer, 0xFF, ED047TC1_WIDTH * ED047TC1_HEIGHT / 2);

}

//Initialize the display
void Ed047TC1::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) printf("Ed047TC1::init(%d)\n", debug);
    
    epd_init();
    epd_poweron();
}

void Ed047TC1::fillScreen(uint16_t color) {
  // Same as: fillRect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color);
  epd_fill_rect(0, 0, ED047TC1_WIDTH, ED047TC1_HEIGHT, color, framebuffer);
}

void Ed047TC1::clearScreen()
{
  epd_clear();
}

void Ed047TC1::clearArea(Rect_t area) {
  epd_clear_area(area);
}

void Ed047TC1::update(enum DrawMode mode)
{
  epd_draw_image(epd_full_screen(), framebuffer, mode);
}

// Retrieve from framebuffer this single pixel color
// Based on: https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp#L888
uint16_t Ed047TC1::getPixel(int16_t x, int16_t y) {
    //Analize how to return this value x,y from buffer
    uint8_t color = 0x33; // white

    if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
      // Pixel is in-bounds. Rotate coordinates if needed
      color = framebuffer[y *width()/2 + x/2];

      // This is definitively not right in this method
      /* if (x % 2) {
        color = (color & 0xF0) | (color & 0xF0);
      } else {
        color = (color & 0xF0) | (color >> 4);
      } */
      _tempalert = false;

    } else {
      if (!_tempalert) {
        printf("getPixel(%d, %d) out of bounds, x or y > screen\n",x,y);
        _tempalert = true;
      }
    }

   if (x%10==0) {
     //printf("x:%d y:%d\n",x,y);
   }
   return color;
}

void Ed047TC1::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  //if (using_rotation) _rotate(x, y, w, h);
  if (x >= ED047TC1_WIDTH) {
    printf("Will not update. x position:%d  is major than display max width:%d\n", x, ED047TC1_WIDTH);
    return;
  }
  if (y >= ED047TC1_HEIGHT) {
    printf("Will not update. y position:%d  is major than display max height:%d\n", y, ED047TC1_HEIGHT);
    return;
  }
  Rect_t area = {
    .x = x,
    .y = y,
    .width = w,
    .height = h,
  };

  uint8_t *buffer = (uint8_t *)heap_caps_malloc(w*h/2,MALLOC_CAP_SPIRAM);
  memset(buffer, 0xFF, w*h/2);

  DrawMode mode = BLACK_ON_WHITE;
  uint16_t i = 0;

  // Not sure if reading the pixels like this is fine. 
  // Fonts come with wind from the right (anti-italics)
  // Send only this area from GFX to our buffer
  for (int16_t y1 = y; y1 <= y+h; y1++)
  {
    for (int16_t x1 = x; x1 <= x+w/2; x1++)
    { 
      // Adafruit getPixel(x,y) color does not exist, redirect to our own:
      // 0xf0 fixed -> square with light gray. Issue is when trying to read the pixel
      buffer[i] = getPixel(x1,y1);
      
      ++i;
    }
  }

  epd_draw_image(area, buffer, mode);
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
    case 1:
      swap(x, y);
      swap(w, h);
      x = width() - x - w - 1;
      break;
    case 2:
      x = width() - x - w - 1;
      y = height() - y - h - 1;
      break;
    case 3:
      swap(x, y);
      swap(w, h);
      y = height() - y - h - 1;
      break;
  }
}

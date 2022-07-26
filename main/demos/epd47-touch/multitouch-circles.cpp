/**
 * This is a demo to be used with EPD47 parallel from Lilygo:
 * https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED047TC1.h
 * 
 * This demo needs idf.py menuconfig > Touch FT6X36 Configuration 
 * Parameter: L58_MULTITOUCH set to 1
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "L58Touch.h"

// NOTE use config-examples/ttgo-EPD47-touch 
//      as your sdkconfig for the right I2C/INT pins
// INTGPIO is touch interrupt, goes HI when it detects a touch, which coordinates are read by I2C
L58Touch ts(CONFIG_TOUCH_INT);
#include "parallel/ED047TC1touch.h"
// Inject touch into display class
Ed047TC1t display(ts);

// Optional font for the writing box:
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>

// Useful shortcuts to find max btw. 2 values
#define new_max(x,y) (((x) >= (y)) ? (x) : (y))
#define new_min(x,y) (((x) <= (y)) ? (x) : (y))

extern "C"
{
   void app_main();
}

uint32_t t_counter = 0;
uint16_t circle_radius = 20;

void touchEvent(TPoint p0, TPoint p1, TEvent e)
{
    ++t_counter;
    display.drawCircle(p0.x, p0.y, circle_radius, EPD_BLACK);
    display.drawCircle(p0.x-1, p0.y-1, circle_radius, EPD_BLACK);
    
    // Only if there is a second touch then draw a second circle
    if (p1.x!=0 && p1.y!=0) {
      display.drawCircle(p1.x, p1.y, circle_radius, EPD_BLACK);
      display.drawCircle(p1.x-1, p1.y-1, circle_radius, EPD_BLACK);
      display.drawLine(p0.x, p0.y, p1.x, p1.y, EPD_BLACK);
      display.drawLine(p0.x-1, p0.y-1, p1.x-1, p1.y-1, EPD_BLACK);
      display.drawLine(p0.x+1, p0.y+1, p1.x+1, p1.y+1, EPD_BLACK);
      uint16_t box_x = new_min(p0.x-circle_radius, p1.x-circle_radius);
      uint16_t box_y = new_min(p0.y-circle_radius, p1.y-circle_radius);
      uint16_t box_w = new_max(p0.x-circle_radius, p1.x-circle_radius)+(circle_radius*2);
      uint16_t box_h = new_max(p0.y-circle_radius, p1.y-circle_radius)+(circle_radius*2);
      display.updateWindow(box_x, box_y, box_w, box_h, MODE_DU);
      printf("x:%d y:%d w:%d h:%d p0.y:%d p1.y:%d\n", box_x, box_y, box_w, box_h, p0.y, p1.y);
    } else {
       // Just make a small update for one
       display.updateWindow(p0.x-circle_radius, p0.y-circle_radius, circle_radius*2, circle_radius*2, MODE_DU);
    }
    // Clear play area after N touch events
    if (t_counter%100 == 0) {
       display.clearScreen();
    }
    
}

void app_main(void)
{

   // Initialize epaper class
   display.init(false);
   display.clearScreen();
   display.setTextColor(EPD_BLACK);
   #ifndef CONFIG_L58_MULTITOUCH
      printf("IMPORTANT: L58_MULTITOUCH need to be enabled in menuconfig for this demo to run properly\nTOUCH_FT6X36 > enable Multitouch");
   #endif
   // Register callback function to receive events
   display.registerMultiTouchHandler(touchEvent);
  
   for (;;) {
      display.touchLoop();
   }
}

/**
 * This is a demo to be used with EPD47 parallel from Lilygo:
 * https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED047TC1.h
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

extern "C"
{
   void app_main();
}

uint32_t t_counter = 0;
uint16_t circle_radius = 20;

void touchEvent(TPoint p, TEvent e)
{

    ++t_counter;
    display.drawCircle(p.x, p.y, circle_radius, EPD_BLACK);
    display.drawCircle(p.x-1, p.y-1, circle_radius, EPD_BLACK);
    display.updateWindow(p.x-circle_radius, p.y-circle_radius, circle_radius*2, circle_radius*2, MODE_DU);

    // Clear play area after 20 touch events
    if (t_counter%20 == 0) {
       display.clearScreen();
    }
    printf("X: %d Y: %d count:%d Ev:%d\n", p.x, p.y, t_counter, int(e));
}

void app_main(void)
{

   // Initialize epaper class
   display.init(false);
   display.clearScreen();
   display.setTextColor(EPD_BLACK);
   // Register callback function to receive events
   display.registerTouchHandler(touchEvent);
  
   for (;;) {
      display.touchLoop();
   }
}

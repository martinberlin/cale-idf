/**
 * This is a demo to be used with EPD47 parallel from Lilygo:
 * https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED047TC1.h
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "L58Touch.h"
#include "parallel/ED047TC1.h"
// INTGPIO is touch interrupt, goes HI when it detects a touch, which coordinates are read by I2C
L58Touch ts(CONFIG_TOUCH_INT);

extern "C"
{
   void app_main();
}

uint32_t t_counter = 0;

void touchEvent(TPoint p, TEvent e)
{

    ++t_counter;
    printf("X: %d Y: %d count:%d Ev:%d\n", p.x, p.y, t_counter, int(e));
}

void registerTouch(void (*fn)(TPoint point, TEvent e))
{
	ts._touchHandler = fn;
   printf("registerTouch called\n");
}

void app_main(void)
{
   bool ret_val = ts.begin(ED047TC1_WIDTH, ED047TC1_HEIGHT);
   // Never arrives here
   printf("ts.begin returns:%d\n", ret_val);
   registerTouch(touchEvent);
   
   //printf("Test: %d ", gpio_get_level((gpio_num_t) CONFIG_TOUCH_INT));
// Just loop and detect touch events
   while(1) {
     ts.loop();
   }
}

#include "driver/gpio.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO

static const char* TAG2 = "benchmark-cpu";

int64_t start;
int64_t after;
int64_t d;

gpio_num_t gpio_blink = BLINK_GPIO;

void blink_test() {
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. 
    */
    gpio_pad_select_gpio(gpio_blink);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(gpio_blink, GPIO_MODE_OUTPUT);
    printf("Speed test\n");
    printf("----------\n");
    printf("GPIO set test\n");
    gpio_set_level(gpio_blink, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    printf("digitalRead: \n");

  start = esp_timer_get_time();

  for (int i=0; i<2; i++)
  {
    for (int j=0; j<10000; j++)
    {
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
      gpio_get_level(gpio_blink);       
    }
  }
  after = esp_timer_get_time();
  d = after-start;
  d /= 10.0;
  ESP_LOGI(TAG2, "GPIO_get_level * 200000: %lld us (digitalRead)", d);
}

void gpio_on_off() {
      start = esp_timer_get_time();

  for (int i=0; i<2; i++)
  {
    for (int j=0; j<10000; j++)
    {
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0);   
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
      gpio_set_level(gpio_blink, 1); 
      gpio_set_level(gpio_blink, 0); 
    }
  }
  after = esp_timer_get_time();
  d = after-start;
  d /= 20.0;
  ESP_LOGI(TAG2, "GPIO_set_level ON/OFF * 200000: %lld us (digitalWrite)", d);

}

void math_multiply() {
     unsigned char c1,c2;
  c1 = 2;
  c2 = 3;
start = esp_timer_get_time();

  for (int i=0; i<20; i++)
  {
    for (int j=0; j<10000; j++)
    {
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
      c1 *= c2;
    }
  }
  after = esp_timer_get_time();
  d = after-start;
  d /= 20.0;
  ESP_LOGI(TAG2, "Multiply byte 200000*20: %lld us", d);
}
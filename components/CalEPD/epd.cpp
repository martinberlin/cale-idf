#include "epd.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

/* 
// Not sure if this should be a generic method that takes timeout as a parameter
void Epd::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>1800000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
} */

size_t Epd::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}

void Epd::print(const std::string& text){
   for(auto c : text) {
     write(uint8_t(c));
   }
}

void Epd::println(const std::string& text){
   for(auto c : text) {
     write(uint8_t(c));
   }
   write(10); // newline
}

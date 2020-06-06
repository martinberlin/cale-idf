#include "epd.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// display.print / println handling
// TODO: Implement printf
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

/* 
// Not sure if this should be a generic method that takes timeout as a parameter
void Epd::_waitBusy(const char* message){
  // Analize it
} 
  // Optionally if we would need to access GFX
  // Adafruit_GFX::setTextColor(color);
*/
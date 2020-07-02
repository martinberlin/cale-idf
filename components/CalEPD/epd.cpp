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
     if (c==195) continue; // Skip to next letter
     // Cope with umlauten - Needs work
     // TODO: Research a smarter way to do this
     switch (c)
     {
     case 164: // ä update to right adafruit GFX char
       c = 228;
       break;
     case 182: // ö
       c = 246;
       break;
     case 188: // ü
       c = 254;
       break;

      case 132: // Ä
       c = 196;
       break;
     case 150: // Ö
       c = 214;
       break;
     case 156: // Ü
       c = 220;
       break;
     } 
     printf("char: %d\n", c);
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
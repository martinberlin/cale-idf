#include "plasticlogic.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include "epd.h"
// Plasticlogic will replace Epd as baseclass for this models and have all common methods for all EPDs of this manufacturer

// display.print / println handling
// TODO: Implement printf
size_t PlasticLogic::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}
uint8_t PlasticLogic::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

void PlasticLogic::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void PlasticLogic::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character when using umlauts and other characters:
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
   write(10); // newline
}

void PlasticLogic::newline() {
  write(10);
}

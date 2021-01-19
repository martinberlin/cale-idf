#include "epdParallel.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// display.print / println handling
// TODO: Implement printf
size_t EpdParallel::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}
uint8_t EpdParallel::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

uint8_t EpdParallel::_unicodePerChar(uint8_t c) {
  // Cope with umlauten - Needs work
     // TODO: Research a smarter way to do this
     switch (c)
     {
       // German characters
     case 164: // ä update to right adafruit GFX char
       c = 228;
       break;
     case 182: // ö
       c += 64;
       break;
     case 188: // ü -> used also in spanish cigüeña
       c += 64;
       break;
     case 132: // Ä
       c += 64;
       break;
     case 150: // Ö
       c += 64;
       break;
     case 156: // Ü
       c += 64;
       break;
     case 159: // ß
       c += 64;
       break;
     // Spanish/French/North EU characters
       case 166: // æ
       c += 64;
       break;
       case 167: // ç
       c += 64;
       break;
       case 168: // è
       c += 64;
       break;
       case 169: // é
       c += 64;
       break;
       case 170: // ê
       c += 64;
       break;
       case 171: // ë
       c += 64;
       break;
       }
       return c;
}

void EpdParallel::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void EpdParallel::print(const char c){
     write(uint8_t(c));
}

void EpdParallel::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character, should be faster and cover more chars
     c = _unicodeEasy(c);
     //c = _unicodePerChar(c); // _unicodePerChar has more control since they are only hand-picked chars
     write(uint8_t(c));
   }
   write(10); // newline
}

void EpdParallel::newline() {
  write(10);
}

/* 
  // Optionally if we would need to access GFX
  // Adafruit_GFX::setTextColor(color);
*/
#include "epd7color.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// display.print / println handling
// TODO: Implement printf
size_t Epd7Color::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}
uint8_t Epd7Color::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

uint8_t Epd7Color::_unicodePerChar(uint8_t c) {
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

void Epd7Color::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void Epd7Color::print(const char c){
     write(uint8_t(c));
}

void Epd7Color::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character, should be faster and cover more chars
     c = _unicodeEasy(c);
     //c = _unicodePerChar(c); // _unicodePerChar has more control since they are only hand-picked chars
     write(uint8_t(c));
   }
   write(10); // newline
}

void Epd7Color::newline() {
  write(10);
}

/**
 * From GxEPD2 (Jean-Marc)
 * Converts from color constants to the right 4 bit pixel color in Acep epaper 7 color
 */
uint8_t Epd7Color::_color7(uint16_t color)
    {
      static uint16_t _prev_color = EPD_BLACK;
      static uint8_t _prev_color7 = 0x00; // black
      if (color == _prev_color) return _prev_color7;
      uint8_t cv7 = 0x00;
      switch (color)
      {
        case EPD_BLACK: cv7 = 0x00; break;
        case EPD_WHITE: cv7 = 0x01; break;
        case EPD_GREEN: cv7 = 0x02; break;
        case EPD_BLUE:  cv7 = 0x03; break;
        case EPD_RED:   cv7 = 0x04; break;
        case EPD_YELLOW: cv7 = 0x05; break;
        case EPD_ORANGE: cv7 = 0x06; break;
        case EPD_PURPLE: cv7 = 0x07; break; 
        default:
          {
            uint16_t red = color & 0xF800;
            uint16_t green = (color & 0x07E0) << 5;
            uint16_t blue = (color & 0x001F) << 11;
            if ((red < 0x8000) && (green < 0x8000) && (blue < 0x8000)) cv7 = 0x00; // black
            
            else if ((red > 0x9000) && (green > 0x8800) && (blue > 0x9000) 
                 && (red < 0xC000) && (green < 0xC000) && (blue < 0xC000) ) cv7 = 0x07; // purple
            else if ((red >= 0x8000) && (green >= 0x8000) && (blue >= 0x8000)) cv7 = 0x01; // white
            
            else if ((red >= 0x8000) && (blue >= 0x8000)) cv7 = red > blue ? 0x04 : 0x03;  // red, blue
            else if ((green >= 0x8000) && (blue >= 0x8000)) cv7 = green > blue ? 0x02 : 0x03; // green, blue
            else if ((red >= 0x8000) && (green >= 0x8000))
            {
              static const uint16_t y2o_lim = ((EPD_YELLOW - EPD_ORANGE) / 2 + (EPD_ORANGE & 0x07E0)) << 5;
              cv7 = green > y2o_lim ? 0x05 : 0x06; // yellow, orange
            }
            else if (red >= 0x8000) cv7 = 0x04; // red
            else if (green >= 0x8000) cv7 = 0x02; // green
            else cv7 = 0x03; // blue
          }
      }
      _prev_color = color;
      _prev_color7 = cv7;
      return cv7;
    }

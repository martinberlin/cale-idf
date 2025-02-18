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
uint8_t Epd::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

uint8_t Epd::_unicodePerChar(uint8_t c) {
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

void Epd::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void Epd::print(const char c){
     write(uint8_t(c));
}

void Epd::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character, should be faster and cover more chars
     c = _unicodeEasy(c);
     //c = _unicodePerChar(c); // _unicodePerChar has more control since they are only hand-picked chars
     write(uint8_t(c));
   }
   write(10); // newline
}

/**
 * @brief Similar to printf
 * Note that buffer needs to end with null character
 * @param format 
 * @param ... va_list
 */
void Epd::printerf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char max_buffer[1024];
    int size = vsnprintf(max_buffer, sizeof max_buffer, format, args);
    va_end(args);

    if (size < sizeof(max_buffer)) {
        print(std::string(max_buffer));
    } else {
      ESP_LOGE("Epd::printerf", "max_buffer out of range. Increase max_buffer!");
    }
}

void Epd::newline() {
  write(10);
}

void Epd::draw_centered_text(const GFXfont *font, int16_t x, int16_t y, uint16_t w, uint16_t h, const char* format, ...) {
    // Handle printf arguments
    va_list args;
    va_start(args, format);
    char max_buffer[1024];
    int size = vsnprintf(max_buffer, sizeof max_buffer, format, args);
    va_end(args);
    string text = "";
    if (size < sizeof(max_buffer)) {
      text = std::string(max_buffer);
      
    } else {
      ESP_LOGE("draw_centered_text", "max_buffer out of range. Increase max_buffer!");
    }
    // Draw external boundary where text needs to be centered in the middle
    setFont(font);
    int16_t text_x = 0;
    int16_t text_y = 0;
    uint16_t text_w = 0;
    uint16_t text_h = 0;

    getTextBounds(text.c_str(), x, y, &text_x, &text_y, &text_w, &text_h);
    // Calculate the middle position
    
    uint16_t ty = (h/2)+y+(text_h/2);
    // Fix for big fonts (>100 pt)
    if (text_h > (height()/3) ) {
      text_x += (w-text_w)/2.2;
      ty -= text_h*1.8;
    } else {
      text_x += (w-text_w)/2;
    }
    //drawRect(text_x, ty-text_h, text_w, text_h, 0); // text boundaries test

    if (text_w > w) {
        printf("W: Text width out of bounds");
    }
    if (text_h > h) {
        printf("W: Text height out of bounds");
    }

    setCursor(text_x, ty);
    print(text);
}

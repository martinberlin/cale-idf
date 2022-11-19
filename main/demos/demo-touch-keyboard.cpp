/**
 * This is a demo to be used with Good Display 2.7 touch epaper 
 * http://www.e-paper-display.com/products_detail/productId=406.html 264*176 px monochome epaper
 * 
 * The difference with the demo-touch.cpp demo is that:
 * 
 * In this demo Epd class gdew027w3T is used.
 * This class expects EpdSPI and FT6X36 to be injected. Meaning that then touch methods
 * can triggered directly from gdew027w3T class and also that would be automatic rotation aware
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"

#include "goodisplay/touch/gdey027T91T.h"
//#include <gdew027w3T.h>
// Optional font for the writing box:
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
bool use_custom_font = true; // false to use default Adafruit GFX font (Small 8px)
#define DEBUG_TOUCH_COUNT 0
#define DEBUG_TOUCH_KEY 0

// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);
EpdSpi io;
Gdey027T91T display(io, ts);
//Gdew027w3T display(io, ts);
uint8_t display_rotation = 3; // 1 or 3: Landscape mode
extern "C"
{
   void app_main();
}
// Turn true to use only partial update when clearing the screen
// Is faster but never cleans completely the draw area like a full update
bool ClearUsesPartialUpdate = false;

// Some GFX constants. Some of them are defined in drawUX()
uint16_t line1_ystart = 90;
uint16_t line2_ystart = 0;
uint16_t line3_ystart = 0;
uint8_t key_width_1 = 0;
uint8_t key_width_2 = 0;
uint16_t epaperTextColor  = EPD_BLACK;
uint16_t epaperBackground = EPD_WHITE;

// Keyboard lines constant
const uint8_t line1_size = 10;
const uint8_t line2_size = 9;
const uint8_t line3_size = 7;
// Arrays to hold the X start of each key
uint16_t line1_x[line1_size];
uint16_t line2_x[line2_size];
uint16_t line3_x[line3_size];
// Keyboard char map for each line
char key_line1[10] = {'Q','W','E','R','T','Z','U','I','O','P'};

char key_line2[9] = {'A','S','D','F','G','H','J','K','L'};

char key_line3[7] = {'Y','X','C','V','B','N','M'};

// Start point of Write area cursor position
uint16_t cursor_x = 10;
uint16_t cursor_y = 20;
uint16_t cursor_y_line_height = 9; // Font dependant
// Measuring the X start of the SPACE key
uint16_t key_space_x = 0;
uint8_t key_space_width = 40;
// Width and height of the area that will be partial updated in the epaper when pressing a Key
// This pixel distance should be relative to the chosen font:
uint8_t cursor_x_offset = 9;
uint8_t cursor_y_offset = 0; 
uint8_t cursor_space = 4; 
uint16_t t_counter = 0;

template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

void writeCarriageReturn(){
  display.setCursor(cursor_x, display.getCursorY()+cursor_y_line_height);
}

/**
 * Draw Keyboard
 */
void drawUX(){
    // Correct this pixels for your font selection of choice
    if (use_custom_font) {
        cursor_x_offset = 15;
        cursor_y_offset = 5;
        cursor_space = 6;
        cursor_y_line_height = 14;
    }
    // key_width_1: Also measures Height for all lines
    key_width_1 = display.width()/sizeof(key_line1);
    key_width_2 = display.width()/sizeof(key_line2);
    uint16_t key_x = 1;
    uint8_t key_x_offset = 9;
    uint16_t key_y = line1_ystart;
    uint8_t impression_idx = 0;
    // LINE 1: Q -> P
    for (auto c:key_line1) {
      display.drawRect(key_x,key_y,key_width_1,key_width_1,EPD_BLACK);
      display.setCursor(key_x+key_x_offset, key_y+10);
      display.print(c);
      line1_x[impression_idx] = key_x;
      key_x+=key_width_1;
      // Space wil take the place of O->P
      if (impression_idx==6) {
        key_space_x = key_x;
      }
      ++impression_idx;
    }
    
    //LINE 2:A -> L
    impression_idx = 0;
    key_x = 1;
    key_y += key_width_1;
    line2_ystart = key_y;
    for (auto c:key_line2) {
      display.drawRect(key_x,key_y,key_width_2,key_width_1,EPD_BLACK);
      display.setCursor(key_x+key_x_offset, key_y+10);
      display.print(c);
      line2_x[impression_idx] = key_x;
      key_x+=key_width_2;
      impression_idx++;
    }

    //LINE 3: Y -> M + SPACE + CLS (Clear Screen)
    impression_idx = 0;
    key_x = 1;
    key_y += key_width_1;
    line3_ystart = key_y;
    for (auto c:key_line3) {
      display.drawRect(key_x,key_y,key_width_1,key_width_1,EPD_BLACK);
      display.setCursor(key_x+key_x_offset, key_y+10);
      display.print(c);
      line3_x[impression_idx] = key_x;
      key_x+=key_width_1;
      impression_idx++;
    }
    
    display.drawRect(key_space_x,key_y,key_space_width,key_width_1,EPD_BLACK);
    display.setCursor(key_x+5, key_y+10);
    display.print("SPACE");

    display.drawRect(key_space_x+key_space_width,key_y,key_space_width-1,key_width_1,EPD_BLACK);
    display.setCursor(key_space_x+key_space_width+7, key_y+10);
    display.print("CLS");

    // Set cursor and default font for the Write area
    if (use_custom_font) {
      display.setFont(&Ubuntu_M8pt8b);
    }
    display.setCursor(cursor_x,cursor_y);
    display.update();
}


void touchEvent(TPoint p, TEvent e)
{
  #if defined(DEBUG_TOUCH_COUNT)
    ++t_counter;
    printf("X: %d Y: %d count:%d Ev:%d\n", p.x, p.y, t_counter, int(e));
  #endif

  // Trigger keys only on TAP
  if (e != TEvent::Tap) return;

  
  // First the Y line IFs
  if (p.y>line1_ystart && p.y<line2_ystart) {         // LINE 1

    for (uint8_t idx = 0; idx < line1_size; idx++) {      
      if (p.x>line1_x[idx] && p.x<line1_x[idx]+key_width_1) {
        // No idea why comes sometimes a strange 255 character (Out of the uint_8?)
        if ((uint8_t)key_line1[idx]<65 || (uint8_t)key_line1[idx]>90) break; 
        #if defined(DEBUG_TOUCH_KEY) && DEBUG_TOUCH_KEY==1
          printf("LINE 1 KEY:%c cX:%d cY:%d | Tx:%d Ty:%d\n",key_line1[idx], display.getCursorX(), display.getCursorY(), p.x, p.y);
        #endif
        // Check if the cursor arrived to the end of the X visible space
        if (display.getCursorX()>display.width()-cursor_x_offset) writeCarriageReturn();
        display.print(key_line1[idx]);
        display.updateWindow(display.getCursorX()-cursor_x_offset, display.getCursorY()-cursor_y_offset, cursor_x_offset, cursor_y_offset);
      }
    }
    
  } else if (p.y>line2_ystart && p.y<line3_ystart) {  // LINE 2

    for (uint8_t idx = 0; idx < line2_size; idx++) {
      if (p.x>line2_x[idx] && p.x<line2_x[idx]+key_width_1) {
        if ((uint8_t)key_line2[idx]<65 || (uint8_t)key_line2[idx]>90) break; 
        
        #if defined(DEBUG_TOUCH_KEY) && DEBUG_TOUCH_KEY==1
          printf("LINE 2 KEY:%c cX:%d cY:%d | Tx:%d Ty:%d\n",key_line2[idx], display.getCursorX(), display.getCursorY(), p.x, p.y);
        #endif
        if (display.getCursorX()>display.width()-cursor_x_offset) writeCarriageReturn();
        display.print(key_line2[idx]);
        display.updateWindow(display.getCursorX()-cursor_x_offset, display.getCursorY()-cursor_y_offset, cursor_x_offset, cursor_y_offset);
      }
    }

  } else if (p.y>line3_ystart) {                    // LINE 3

    for (uint8_t idx = 0; idx < line3_size; idx++) {
      if (p.x>line3_x[idx] && p.x<line3_x[idx]+key_width_1) {
        if ((uint8_t)key_line3[idx]<65 || (uint8_t)key_line3[idx]>90) break;
        #if defined(DEBUG_TOUCH_KEY) && DEBUG_TOUCH_KEY==1
          printf("LINE 3 KEY:%c cX:%d cY:%d | Tx:%d Ty:%d\n",key_line3[idx], display.getCursorX(), display.getCursorY(), p.x, p.y);
        #endif
        if (display.getCursorX()>display.width()-cursor_x_offset) writeCarriageReturn();
        display.print(key_line3[idx]);
        display.updateWindow(display.getCursorX()-cursor_x_offset, display.getCursorY()-cursor_y_offset, cursor_x_offset, cursor_y_offset);
      }
    }
    if (p.x>key_space_x && p.x<key_space_x+key_space_width) {
      #if defined(DEBUG_TOUCH_KEY) && DEBUG_TOUCH_KEY==1
        printf("SPACE Key\n");
      #endif
        display.setCursor(display.getCursorX()+cursor_space, display.getCursorY());
    }
    if (p.x>key_space_x+key_space_width) {
      #if defined(DEBUG_TOUCH_KEY) && DEBUG_TOUCH_KEY==1
        printf("CLS Key\n");
      #endif

      if (ClearUsesPartialUpdate) {
        display.fillRect(0,0,display.width(),line1_ystart,EPD_WHITE);
        display.updateWindow(0,0,display.width()-1,line1_ystart);
        // Reset cursors
        cursor_x = 10;
        cursor_y = 20;
        display.setCursor(cursor_x,cursor_y);
        } else {
          esp_restart();
        }
    }
  }
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);
   display.fillScreen(epaperBackground);
   display.setTextColor(epaperTextColor);
   // displayRotation includes both epaper + touch rotation
   display.displayRotation(display_rotation);
   printf("display.colors_supported:%d display.rotation: %d\n", display.colors_supported,display.getRotation());  
   drawUX();
  
   display.registerTouchHandler(touchEvent);
  
  for (;;) {
    display.touchLoop();
  }
}

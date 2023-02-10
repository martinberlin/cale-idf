/**
 * This is a demo to be used with Good Display 2.7 touch epaper 
 */ 

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"
#include "driver/gpio.h"

//#include "gdew027w3.h"
#include "goodisplay/gdey027T91.h"
// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);
EpdSpi io;
//Gdew027w3 display(io);
//Gdey029T94 display(io);
Gdey027T91 display(io);

// Only debugging:
//#define DEBUG_COUNT_TOUCH

// Relay Latch (high) / OFF
#define GPIO_RELAY_ON 1
#define GPIO_RELAY_OFF 3
// Pulse to move the switch in millis
const uint16_t signal_ms = 50;
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
uint8_t display_rotation = 2;

extern "C"
{
   void app_main();
}
void delay(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }
// Some GFX constants
uint16_t blockWidth = 42;
uint16_t blockHeight = display.height()/4;
uint16_t lineSpacing = 18;
uint16_t circleColor = EPD_BLACK;
uint16_t circleRadio = 10;
uint16_t selectTextColor  = EPD_WHITE;
uint16_t selectBackground = EPD_BLACK;
template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

bool switch_state = false; // starts false = OFF

void draw_centered_text(const GFXfont *font, char * text, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    // Draw external boundary where text needs to be centered in the middle
    //printf("drawRect x:%d y:%d w:%d h:%d\n\n", x, y, w, h);
    display.drawRect(x, y, w, h, EPD_DARKGREY);

    display.setFont(font);
    int16_t text_x = 0;
    int16_t text_y = 0;
    uint16_t text_w = 0;
    uint16_t text_h = 0;

    display.getTextBounds(text, x, y, &text_x, &text_y, &text_w, &text_h);
    //printf("text_x:%d y:%d w:%d h:%d\n\n", text_x,text_y,text_w,text_h);
    //display.drawRect(text_x, text_y, text_w, text_h, EPD_BLACK); // text boundaries

    if (text_w > w) {
        printf("W: Text width out of bounds");
    }
    if (text_h > h) {
        printf("W: Text height out of bounds");
    }
    // Calculate the middle position
    text_x += (w-text_w)/2;

    uint ty = (h/2)+y+(text_h/2);
    display.setCursor(text_x, ty);
    display.print(text);
}

void switchState(bool state) {
  if (state) {
    gpio_set_level((gpio_num_t)GPIO_RELAY_ON, 1); // OFF
    delay(signal_ms);
    gpio_set_level((gpio_num_t)GPIO_RELAY_ON, 0); // OFF release
  } else {
    gpio_set_level((gpio_num_t)GPIO_RELAY_OFF, 1); // OFF
    delay(signal_ms);
    gpio_set_level((gpio_num_t)GPIO_RELAY_OFF, 0); // OFF release
  }
}

void drawUX(){
  uint16_t dw = display.width();
  uint16_t dh = display.height();
  uint8_t  sw = 20;
  uint8_t  sh = 50;
  uint8_t  keyw = 16;
  uint8_t  keyh = 20;
  display.fillScreen(EPD_WHITE);
  display.drawRoundRect(dw/2-sw/2, dh/2-sh/2, sw, sh, 4, EPD_BLACK);

  // OFF position
  if (!switch_state) {
    display.fillRoundRect(dw/2-keyw/2, dh/2, keyw, keyh, 5, EPD_BLACK);
    gpio_set_level((gpio_num_t)GPIO_RELAY_OFF, 1); // OFF
    delay(signal_ms);
    gpio_set_level((gpio_num_t)GPIO_RELAY_OFF, 0); // OFF release
    printf("Draw OFF\n\n");
  } else {
    display.fillRoundRect(dw/2-keyw/2, dh/2-keyh, keyw, keyh, 5, EPD_BLACK);
    gpio_set_level((gpio_num_t)GPIO_RELAY_ON, 1); // ON
    delay(signal_ms);
    gpio_set_level((gpio_num_t)GPIO_RELAY_ON, 0); // ON release
    printf("Draw ON\n\n");
  }
  
  char * label = (switch_state) ? (char *)"ON" : (char *)"OFF";
  draw_centered_text(&Ubuntu_M8pt8b, label, dw/2-22, dh/2-sh, 40, 20);
  display.update();
  // It does not work correctly with partial update leaves last position gray
  //display.updateWindow(dw/2-40, dh/2-keyh-40, 100, 86);
}


uint16_t t_counter = 0;

void touchEvent(TPoint p, TEvent e)
{
  #if defined(DEBUG_COUNT_TOUCH) && DEBUG_COUNT_TOUCH==1
    ++t_counter;
    printf("e %x %d  ",e,t_counter); // Working
  #endif

  if (e != TEvent::Tap && e != TEvent::DragStart && e != TEvent::DragMove && e != TEvent::DragEnd)
    return;

  switch_state = !switch_state;
  drawUX();
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);

   //Initialize GPIOs direction & initial states
  gpio_set_direction((gpio_num_t)GPIO_RELAY_ON, GPIO_MODE_OUTPUT);
  gpio_set_direction((gpio_num_t)GPIO_RELAY_OFF, GPIO_MODE_OUTPUT);
  switchState(false); // OFF at the beginning

  //DEBUG turning on/off the Relay to make sure it ticks (just change to true)
  while(false) {
    switchState(true);
    delay(1500);
    switchState(false);
    delay(1500);
  }

   // Test Epd class
   display.init(false);
   display.setRotation(display_rotation);
   //display.update();
   display.setFont(&Ubuntu_M8pt8b);
   display.setTextColor(EPD_BLACK);
   drawUX();
   
  // Instantiate touch. Important pass here the 3 required variables including display width and height
   ts.begin(FT6X36_DEFAULT_THRESHOLD, display.width(), display.height());
   ts.setRotation(display.getRotation());
   ts.registerTouchHandler(touchEvent);
  
    for (;;) {
        ts.loop();
      }
      
}

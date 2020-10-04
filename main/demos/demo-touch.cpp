#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"
#include "soc/rtc_wdt.h"
//#include <gdew027w3.h>
#include <gdew027w3T.h>

// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);

EpdSpi io;
Gdew027w3T display(io);
//Gdew027w3 display(io); // Does not work correctly yet - moved to /fix

// Only debugging:
//#define DEBUG_COUNT_TOUCH
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
uint8_t display_rotation = 0;

extern "C"
{
   void app_main();
}

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

void drawUX(){
    // when it's vertical orientation 
    if (display.getRotation()==1 || display.getRotation()==3)
    {
      //swap(blockWidth,blockHeight);
      blockWidth = display.width()/4;
      blockHeight = 42;
    } else {
      blockHeight = display.height()/4;
      blockWidth = 42;
    }
    if (circleRadio==10) {
      selectTextColor  = EPD_WHITE;
      selectBackground = EPD_BLACK;
    } else {
      selectTextColor  = EPD_BLACK;
      selectBackground = EPD_WHITE;
    }
    display.setTextColor(selectTextColor);
    display.fillRoundRect(1,1,blockWidth,blockHeight,8,selectBackground);
    display.setCursor(5,blockHeight-(lineSpacing));
    display.println("10px");

    if (circleRadio==20) {
      selectTextColor  = EPD_WHITE;
      selectBackground = EPD_BLACK;
    } else {
      selectTextColor  = EPD_BLACK;
      selectBackground = EPD_WHITE;
    }
    display.fillRoundRect(1,blockHeight,blockWidth,blockHeight,8,selectBackground);
    display.drawRoundRect(1,blockHeight,blockWidth,blockHeight,8,selectTextColor);
    display.setTextColor(selectTextColor);
    display.setCursor(5,blockHeight*2-(lineSpacing));
    display.println("20px");

    display.fillRoundRect(1,blockHeight*2,blockWidth,blockHeight,4,EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(5,blockHeight*3-lineSpacing);
    display.println("CLS");

    // 200
    display.fillRoundRect(1,blockHeight*3,blockWidth,blockHeight,8,EPD_WHITE);
    display.drawRoundRect(1,blockHeight*3,blockWidth,blockHeight,8,EPD_BLACK);
    display.setTextColor(EPD_BLACK);
    display.setCursor(5,blockHeight*4-lineSpacing);
    display.print("ROT");
    
}
uint16_t t_counter = 0;

void touchEvent(TPoint p, TEvent e)
{
  #if defined(DEBUG_COUNT_TOUCH) && DEBUG_COUNT_TOUCH==1
    ++t_counter;
    ets_printf("e %x %d  ",e,t_counter); // Working
  #endif

  if (e != TEvent::Tap && e != TEvent::DragStart && e != TEvent::DragMove && e != TEvent::DragEnd)
    return;

  std::string eventName = "";
  switch (e)
  {
  case TEvent::Tap:
    eventName = "tap";
    break;
  case TEvent::DragStart:
    eventName = "DragStart";
    break;
  case TEvent::DragMove:
    eventName = "DragMove";
    break;
  case TEvent::DragEnd:
    eventName = "DragEnd";
    break;
  default:
    eventName = "UNKNOWN";
    break;
  }

  printf("X: %d Y: %d E: %s\n", p.x, p.y, eventName.c_str());
  // Button coordinates need to be adapted depending on rotation
  uint16_t button4_max = 198;
  uint16_t button4_min = 132;
  uint16_t button3 = 66;
  if (display.getRotation()==1 || display.getRotation()==3)
    {
      uint8_t blocks = display.height()/4;
      button4_max = blocks*3;
      button4_min = blocks*2;
      button3 = blocks;
    } 

  if (p.x<blockWidth && p.y>button4_max) { // Rotate 90 degrees
    if (display.getRotation()>3) {
      display_rotation=0;
    } else {
      display_rotation++;
    }
    display.fillScreen(EPD_WHITE);
    display.setRotation(display_rotation);
    ts.setRotation(display_rotation);
    drawUX();
    display.update();
  } else if (p.x<blockWidth && p.y<button4_max && p.y>button4_min) { // Clear screen to black
    display.fillScreen(EPD_BLACK);
    circleColor = EPD_WHITE;
    drawUX();
    display.update();
  } else if (p.x<blockWidth && p.y<button4_min && p.y>button3) { // Set circle color to white
    circleRadio = 20;
  } else if (p.x<blockWidth && p.y<button3 && p.y>1) {
    circleRadio = 10;

  } else {
    // Print a small circle in selected color
    display.fillCircle(p.x, p.y, circleRadio, circleColor);
    display.updateWindow(p.x-circleRadio, p.y-circleRadio, circleRadio*2, circleRadio*2+1);
  }
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);
   //display.setFont(&Ubuntu_M8pt8b);
   //display.setRotation(2);
   
   printf("display.colors_supported:%d\n", display.colors_supported);  
   drawUX();
   display.setRotation(display_rotation);
   display.update();
   
   // Instantiate touch 
   ts.begin();
   ts.setTouchHeight(display.height());
   ts.setTouchWidth(display.width());
   ts.setRotation(display.getRotation());
   ts.registerTouchHandler(touchEvent);
  
    uint8_t c=0;
    for (;;) {
        ts.loop();
      }
      
}

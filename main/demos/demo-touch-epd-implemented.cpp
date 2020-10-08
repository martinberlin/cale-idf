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
#include <gdew027w3T.h>

// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);
EpdSpi io;
Gdew027w3T display(io, ts);

// Only debugging:
//#define DEBUG_COUNT_TOUCH 1
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

  #if defined(DEBUG_COUNT_TOUCH)
  printf("X: %d Y: %d E: %s\n", p.x, p.y, eventName.c_str());
  #endif
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
    printf("Rotation pressed. display.getRotation() %d\n", display.getRotation());
    if (display.getRotation()==3) {
      display_rotation=0;
    } else {
      display_rotation++;
    }
    display.fillScreen(EPD_WHITE);
    // We don't use method setRotation but instead displayRotation that rotates both eink drawPixel & touch coordinates
    display.displayRotation(display_rotation);
    drawUX();
    display.update();

  } else if (p.x<blockWidth && p.y<button4_max && p.y>button4_min) { // Clear screen to white, black eink
    printf("CLS pressed\n");
    display.fillScreen(EPD_WHITE);
    circleColor = EPD_BLACK;
    drawUX();
    display.update();
  } else if (p.x<blockWidth && p.y<button4_min && p.y>button3) { // Set circle color to white
    printf("20px radio pressed\n");
    circleRadio = 20;
  } else if (p.x<blockWidth && p.y<button3 && p.y>1) {
    circleRadio = 10;

  } else {
    printf("Draw a %d px circle\n", circleRadio);
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
   // Optional font setting, empty picks small default
   //display.setFont(&Ubuntu_M8pt8b);

   // displayRotation includes both epaper + touch rotation
   display.displayRotation(display_rotation);
   
   printf("display.colors_supported:%d display.rotation: %d\n", display.colors_supported,display_rotation);  
   drawUX();
  
   display.update();
   
   // Instantiate touch. In this class touch is already instantiated on the display init
   // So this is not needed:
   // ts.begin(FT6X36_DEFAULT_THRESHOLD, display.width(), display.height());

   display.registerTouchHandler(touchEvent);
  
  for (;;) {
    display.touchLoop();
  }
}

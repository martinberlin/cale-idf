/**
 * This is a demo to be used with EPD47 parallel from Lilygo:
 * https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED047TC1.h
 * 
 * The touch awareness of rotation is not working OK for rotation 1 & 3
 * Still did not discovered why, if you do just make a pull request!
 * 
 * This class expects L58Touch to be injected. Meaning that then touch methods
 * can triggered directly from gdew027w3T class and also that would be automatic rotation aware
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "L58Touch.h"

// INTGPIO is touch interrupt, goes HI when it detects a touch, which coordinates are read by I2C
L58Touch ts(CONFIG_TOUCH_INT);
#include "parallel/ED047TC1touch.h"
Ed047TC1t display(ts);

// Only debugging:
#define DEBUG_COUNT_TOUCH 1
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
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
      blockHeight = display.height()/4;
      blockWidth = 100;
    } else {
      blockHeight = display.height()/4;
      blockWidth = 90;
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

  if (e != TEvent::Tap)
    return;

  std::string eventName = "";
  // Only Tap supported for now
  switch (e)
  {
  case TEvent::Tap:
    eventName = "tap";
    break;
  default:
    eventName = "UNKNOWN";
    break;
  }

  #if defined(DEBUG_COUNT_TOUCH)
  printf("X: %d Y: %d E: %s\n", p.x, p.y, eventName.c_str());
  #endif
  
   uint16_t button4_max = blockHeight*3;
   uint16_t button4_min = blockHeight*2;
   uint16_t button3 = blockHeight;

  if (p.x<blockWidth && p.y>button4_max) { // Rotate 90 degrees
    if (display.getRotation()==3) {
      display_rotation=0;
    } else {
      display_rotation++;
    }

    // We don't use method setRotation but instead displayRotation that rotates both eink drawPixel & touch coordinates
    display.displayRotation(display_rotation);
    printf("Rotation pressed. display.setRotation(%d)\n", display.getRotation());
    display.fillScreen(EPD_WHITE);
    display.clearScreen();
    drawUX();
    display.update();

  } else if (p.x<blockWidth && p.y<button4_max && p.y>button4_min) { // Clear screen to white, black eink
    printf("CLS pressed\n");
    display.fillScreen(EPD_WHITE);
    display.clearScreen();
    circleColor = EPD_BLACK;
    drawUX();
    display.update();
    
  } else if (p.x<blockWidth && p.y<button4_min && p.y>button3) { // Set circle color to white
    printf("20px pressed. No tap simulation\n");
    circleRadio = 20;
    ts.tapSimulationEnabled = false;

  } else if (p.x<blockWidth && p.y<button3 && p.y>1) {
    printf("10px pressed. Tap simulation enabled\n");
    circleRadio = 10;
    ts.tapSimulationEnabled = true;

  } else {
    printf("Draw a %d px circle\n", circleRadio);
    // ON 1 & 3 rotation mode resets still did not discovered why
    // And I guess is because Y, Radius are not pair for partial
    uint16_t normX = (p.x %2 == 0) ?p.x:p.x++;
    // Only X seems to be the issue
    //uint16_t normY = (p.y %2 == 0) ?p.y:p.y++;
    uint16_t normR = (circleRadio %2 ==0) ? circleRadio : circleRadio+1;
    display.fillCircle(p.x, p.y, normR, circleColor);
    display.updateWindow(normX-normR, p.y-normR, normR*2+2, normR*2+2);
  }
}

void app_main(void)
{
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Test Epd class
   display.init(false);
   display.setFont(&Ubuntu_M16pt8b);
   display.clearScreen();
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

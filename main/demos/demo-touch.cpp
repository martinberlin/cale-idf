#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FT6X36.h"
#include "soc/rtc_wdt.h"
#include <gdew027w3.h>

// INTGPIO is touch interrupt, goes low when it detects a touch, which coordinates are read by I2C
FT6X36 ts(CONFIG_TOUCH_INT);

EpdSpi io;
Gdew027w3 display(io);
//Gdeh0213b73 display(io); // Does not work correctly yet - moved to /fix

// Only debugging:
//#define DEBUG_COUNT_TOUCH
// FONT used for title / message body - Only after display library
//Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252
#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>

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

void drawUX(){
    display.fillRoundRect(1,1,blockWidth,blockHeight,8,EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(5,blockHeight-(lineSpacing*2));
    display.println("BLACK");display.println(" CIRCLE");

    display.fillRoundRect(1,blockHeight,blockWidth,blockHeight,8,EPD_WHITE);
    display.drawRoundRect(1,blockHeight,blockWidth,blockHeight,8,EPD_BLACK);
    display.setTextColor(EPD_BLACK);
    display.setCursor(5,blockHeight*2-lineSpacing);
    display.println("CIRCLE");

    display.fillRoundRect(1,blockHeight*2,blockWidth,blockHeight,4,EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(5,blockHeight*3-lineSpacing);
    display.println("CLEAR");

    // 200
    display.fillRoundRect(1,blockHeight*3,blockWidth,blockHeight,8,EPD_WHITE);
    display.drawRoundRect(1,blockHeight*3,blockWidth,blockHeight,8,EPD_BLACK);
    display.setTextColor(EPD_BLACK);
    display.setCursor(5,blockHeight*4-lineSpacing);
    display.print("CLEAR");
    
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

  if (p.x<blockWidth && p.y>198) { // Clear screen to white
    display.fillScreen(EPD_WHITE);
    circleColor = EPD_BLACK;
    drawUX();
    display.update();
  } else if (p.x<blockWidth && p.y<198 && p.y>132) { // Clear screen to black
    display.fillScreen(EPD_BLACK);
    circleColor = EPD_WHITE;
    drawUX();
    display.update();
  } else if (p.x<blockWidth && p.y<132 && p.y>66) { // Set circle color to white
    circleColor = EPD_WHITE;
  } else if (p.x<blockWidth && p.y<66 && p.y>1) {
    circleColor = EPD_BLACK;

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

   //display.setRotation(2);

   uint16_t foregroundColor = EPD_WHITE;
   // Make some rectangles showing the different colors or grays
   if (display.colors_supported>1) {
      printf("display.colors_supported:%d\n", display.colors_supported);
      foregroundColor = EPD_RED;
   }

   drawUX();
   display.update();
  
   ts.begin();
   ts.registerTouchHandler(touchEvent);
   
   if (true) {
    uint8_t c=0;
    while (true) {
            ts.loop();

            vTaskDelay(10/portTICK_PERIOD_MS);
            if (c%8) {
              rtc_wdt_feed();
            }
            ++c;
          }
      }
}

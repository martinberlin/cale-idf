#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#include <gdew027w3.h>
#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

EpdSpi io;
Gdew027w3 display(io);

// Time query: HHmm  -> 0800 (8 AM)
const char* timequery = "http://fs.fasani.de/api/?q=date&timezone=Europe/Berlin&f=Hi";

// Clock will refresh each N minutes
int sleepMinutes = 5;

extern "C"
{
   void app_main();
}

void deepsleep(){
    esp_deep_sleep(1000000LL * 60 * sleepMinutes);
}

void app_main(void)
{

   printf("Demo sleeping clock\n");
   // Get initial time from internet 

   // Test Epd class with partial display
   display.init(true);
   display.setRotation(3);
   display.setFont(&Ubuntu_M20pt8b);
   display.setTextColor(EPD_BLACK);
   display.setCursor(80,50);
   display.print("30 Jun");

   display.update();
   // TODO Fix partial update for this epaper:
   //display.updateWindow(90, 80, 100, 30, true);
   display.setCursor(90,80);
   display.print("06:05");
   display.update();
   vTaskDelay(pdMS_TO_TICKS(1000));

   printf("deepsleep for %d minutes\n", sleepMinutes);
   deepsleep();
}

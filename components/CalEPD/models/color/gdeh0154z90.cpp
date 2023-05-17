#include "color/gdeh0154z90.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
// Partial Update Delay
#define GDEH0154Z90_PU_DELAY 300

// Constructor
Gdeh0154z90::Gdeh0154z90(EpdSpi &dio) : Adafruit_GFX(GDEH0154Z90_WIDTH, GDEH0154Z90_HEIGHT),
                                        Epd(GDEH0154Z90_WIDTH, GDEH0154Z90_HEIGHT), IO(dio)
{
    ESP_LOGI(TAG, "Gdeh0154z90() %d*%d\n", GDEH0154Z90_WIDTH, GDEH0154Z90_HEIGHT);
}

//Initialize the display
void Gdeh0154z90::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled)
    {
        ESP_LOGI(TAG, "Gdeh0154z90::init(%d)", debug);
    }
    IO.init(4, debug); // 4MHz frequency

    fillScreen(EPD_WHITE);
}

void Gdeh0154z90::fillScreen(uint16_t color)
{
    uint8_t black = GDEH0154Z90_8PIX_WHITE;
    uint8_t red = GDEH0154Z90_8PIX_RED_WHITE;

    if (color == EPD_BLACK)
    {
        black = GDEH0154Z90_8PIX_BLACK;
    }
    else if (color == EPD_RED)
    {
        red = GDEH0154Z90_8PIX_RED;
    }

    for (uint16_t x = 0; x < sizeof(_black_buffer); x++)
    {
        _black_buffer[x] = black;
        _red_buffer[x] = red;
    }

    if (debug_enabled)
    {
        ESP_LOGI(TAG, "fillScreen(%d) _buffer len:%d", color, sizeof(_black_buffer));
    }
}

void Gdeh0154z90::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    {
        return;
    }

    // check rotation, move pixel around if necessary
    switch (getRotation())
    {
    case 1:
        swap(x, y);
        x = GDEH0154Z90_WIDTH - x - 1;
        break;
    case 2:
        x = GDEH0154Z90_WIDTH - x - 1;
        y = GDEH0154Z90_HEIGHT - y - 1;
        break;
    case 3:
        swap(x, y);
        y = GDEH0154Z90_HEIGHT - y - 1;
        break;
    }
    uint16_t i = x / 8 + y * GDEH0154Z90_WIDTH / 8;

    switch (color)
    {
    case EPD_BLACK:
        _red_buffer[i] = (_red_buffer[i] & (GDEH0154Z90_8PIX_RED_WHITE ^ (1 << (7 - x % 8)))); // white
        _black_buffer[i] = (_black_buffer[i] & (GDEH0154Z90_8PIX_WHITE ^ (1 << (7 - x % 8)))); // white
        break;
    case EPD_WHITE:
        _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
        _red_buffer[i] = (_red_buffer[i] & (GDEH0154Z90_8PIX_RED_WHITE ^ (1 << (7 - x % 8)))); // white
        break;
    case EPD_RED:
        _red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
        _black_buffer[i] = (_black_buffer[i] & (GDEH0154Z90_8PIX_WHITE ^ (1 << (7 - x % 8)))); // white
        break;
    }
}

void Gdeh0154z90::update()
{
    uint64_t startTime = esp_timer_get_time();
    _wakeUp();

    IO.cmd(0x24); // Write RAM for black(0)/white (1)
    uint8_t xLineBytes = GDEH0154Z90_WIDTH / 8;
    uint8_t x1buf[xLineBytes];

    for (uint16_t y = GDEH0154Z90_HEIGHT; y > 0; y--)
      {
        for (uint16_t x = 0; x < xLineBytes; x++)
        {
          uint16_t idx = y * xLineBytes + x;  
          x1buf[x] = (idx < sizeof(_black_buffer)) ? _black_buffer[idx] : 0xFF;
        }
        // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }

    IO.cmd(0x26); // Write RAM for red(1)/white (0)
    for (uint16_t y = GDEH0154Z90_HEIGHT; y > 0; y--)
    {
        for (uint16_t x = 0; x < xLineBytes; x++)
        {
            uint16_t idx = y * xLineBytes + x;  
            x1buf[x] = (idx < sizeof(_red_buffer)) ? _red_buffer[idx] : GDEH0154Z90_8PIX_RED_WHITE;
        }
        // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
    }

    uint64_t endTime = esp_timer_get_time();

    IO.cmd(0x22); //Display Update Control
    IO.data(0xf7);
    IO.cmd(0x20); //Activate Display Update Sequence
    _waitBusy("_Update_Full");

    uint64_t updateTime = esp_timer_get_time();

    ESP_LOGI(TAG, "\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
             (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);

    _sleep();
}

void Gdeh0154z90::_wakeUp()
{
    IO.reset(10);
    _waitBusy("epd_wakeup reset");

    IO.cmd(0x12); // SWRESET
    _waitBusy("epd_wakeup swreset");

    IO.cmd(0x01); // Driver output control
    IO.data(0xC7);
    IO.data(0x00);
    IO.data(0x00);

    IO.cmd(0x11); //data entry mode
    IO.data(0x01);

    IO.cmd(0x44); //set Ram-X address start/end position
    IO.data(0x00);
    IO.data(0x18); //0x18-->(24+1)*8=200

    IO.cmd(0x45);  //set Ram-Y address start/end position
    IO.data(0xC7); //0xC7-->(199+1)=200
    IO.data(0x00);
    IO.data(0x00);
    IO.data(0x00);

    IO.cmd(0x3C); // BorderWavefrom
    IO.data(0x05);
    IO.cmd(0x18); // Read built-in temperature sensor
    IO.data(0x80);
}

void Gdeh0154z90::_sleep()
{
    IO.cmd(0x10); // enter deep sleep
    IO.data(0x01);
}

void Gdeh0154z90::_waitBusy(const char *message)
{
    if (debug_enabled)
    {
        ESP_LOGI(TAG, "_waitBusy for %s", message);
    }
    int64_t time_since_boot = esp_timer_get_time();

    while (1)
    {
        // On low is not busy anymore
        if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0)
            break;
        vTaskDelay(1);
        if (esp_timer_get_time() - time_since_boot > 7000000)
        {
            if (debug_enabled)
            {
                ESP_LOGI(TAG, "Busy Timeout");
            }
            break;
        }
    }
}

void Gdeh0154z90::_rotate(int16_t &x, int16_t &y, int16_t &w, int16_t &h)
{
    switch (getRotation())
    {
    case 1:
        swap(x, y);
        swap(w, h);
        x = GDEH0154Z90_WIDTH - x - w - 1;
        break;
    case 2:
        x = GDEH0154Z90_WIDTH - x - w - 1;
        y = GDEH0154Z90_HEIGHT - y - h - 1;
        break;
    case 3:
        swap(x, y);
        swap(w, h);
        y = GDEH0154Z90_HEIGHT - y - h - 1;
        break;
    }
}

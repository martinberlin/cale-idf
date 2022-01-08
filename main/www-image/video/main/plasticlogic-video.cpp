/**
 * plasticlogic.com Demo of slow RAW RGB565 video read from SPIFFs
 * Inspired by this blogpost: https://appelsiini.net/2020/esp32-mjpeg-video-player/
 * 
 * To read about SPIFFs
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
 * 
 * Video must be encoded in RGB4 pixel format like this:
 * ffmpeg -t 2 -i input.mp4 -vf "fps=15,scale=-1:124:flags=lanczos,crop=220:in_h:(in_w-220)/2:0,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -c:v rawvideo -pix_fmt rgb4 output.rgb
 * OR shorter:
 * ffmpeg -t 2 -i input.mp4 -vf "fps=10,crop=220" -c:v rawvideo -pix_fmt rgb4 output.rgb
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "video";

const char* video_file = "/spiffs/output.rgb";
uint16_t video_width = 220;
uint16_t video_height = 124;
// 4BPP : 2 pixels per byte 
const size_t FRAME_SIZE = video_width/2 * video_height;

#include <plasticlogic021.h>
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
// Plasticlogic EPD should implement EpdSpi2Cs Full duplex SPI
EpdSpi2Cs io;
PlasticLogic021 display(io);

extern "C"
{
   void app_main();
}

void app_main(void)
{
  printf("CalEPD version: %s for Plasticlogic.com\nVIDEO demo\n", CALEPD_VERSION);

  ESP_LOGI(TAG, "Initializing SPIFFS and allocating %d bytes for each video frame\nHEAP: %d", FRAME_SIZE, xPortGetFreeHeapSize());
  uint8_t *videobuffer = (uint8_t*)malloc(FRAME_SIZE);

  esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
  };

  // Use settings defined above to initialize and mount SPIFFS filesystem.
  // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed to mount or format filesystem");
      } else if (ret == ESP_ERR_NOT_FOUND) {
          ESP_LOGE(TAG, "Failed to find SPIFFS partition");
      } else {
          ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
      }
      return;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
  } else {
      ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
  
  // Initialize display class
  display.init();         // Add init(true) for debug
  display.clearScreen();

  ESP_LOGI(TAG, "Reading video. FRAMESIZE: %d\nFree HEAP %d", FRAME_SIZE, xPortGetFreeHeapSize());

  // Open for reading hello.txt
  FILE* fp = fopen(video_file, "r");
  if (fp == NULL) {
      ESP_LOGE(TAG, "Failed to open %s", video_file);
      return;
  }

  uint16_t frame_nr = 0;
  uint16_t y_line = 0;
  uint16_t x_pix_read = 0;

  while (fread(videobuffer, FRAME_SIZE, 1, fp)) {
      frame_nr++;

      for (uint32_t bp = 0; bp < FRAME_SIZE; ++bp) {
        if (x_pix_read > video_width-1) {
          x_pix_read = 0;
          y_line++;
          if (y_line > video_height) break;
        }
        uint8_t low_bits =  videobuffer[bp] & 0x0F;
        uint8_t high_bits = videobuffer[bp] >> 4;
        
        //printf("x %d\n", x_pix_read);
        
        display.drawPixel(x_pix_read, y_line, low_bits>>2);
        display.drawPixel(x_pix_read+1, y_line, high_bits>>2);
        x_pix_read+=2;
      }

      y_line = 0;
      //printf("F%d chk %lld\n", frame_nr, checksum);
      display.update(EPD_UPD_PART);   //EPD_UPD_PART
      //vTaskDelay(20 / portTICK_PERIOD_MS);
      //if (frame_nr == 15) break;
  };

  ESP_LOGI(TAG, "Reached last video frame");
  fclose(fp); 
}
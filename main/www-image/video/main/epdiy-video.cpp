/**
 * EPDiy parallel demo for RAW RGB 4 bit encoded video read from SPIFFs
 * Inspired by this blogpost: https://appelsiini.net/2020/esp32-mjpeg-video-player/
 * 
 * To read about SPIFFs
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
 * 
 * Video must be encoded in RGB4 pixel format like this:
 * ffmpeg -t 2 -i input.mp4 -vf "fps=15,scale=-1:124:flags=lanczos,crop=220:in_h:(in_w-220)/2:0,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -c:v rawvideo -pix_fmt rgb4 output.rgb
 * OR shorter w/ start    end time:
 * ffmpeg -ss 00:00:00 -t 00:00:04 -t 2 -i input.mp4 -vf "fps=6,crop=440" -c:v rawvideo -pix_fmt rgb4 output.rgb
 * OR respecting video size (Attention file might be large!)
 * ffmpeg -ss 00:00:21 -t 00:00:30 -t 2 -i input.mp4 -vf "fps=5,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -c:v rawvideo -pix_fmt rgb4 output.rgb
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_timer.h"

static const char *TAG = "video";
// Alternative video: 
const char* video_file = "/spiffs/output.rgb"; // bunny-RGB4-220x124  output
// NOTE: This is RAW video so the Width must match exactly:
uint16_t video_width = 640;
uint16_t video_height = 326;
// 4BPP : 2 pixels per byte 
const size_t FRAME_SIZE = video_width/2 * video_height;

#include <plasticlogic021.h>
#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
// Only for parallel epaper displays driven by I2S DataBus (No SPI)
// NOTE: This needs Epdiy component https://github.com/vroland/epdiy
// Run idf.py menuconfig-> Component Config -> E-Paper driver and select:
// Display type: LILIGO 4.7 ED047TC1
// Board: LILIGO T5-4.7 Epaper
// In the same section Component Config -> ESP32 Specifics -> Enable PSRAM
#include "parallel/ED047TC1.h"
Ed047TC1 display;
// VIDEO MODES
// Withouth rotation but faster doing a memcpy of entire ROWs
#define FRAMEBUFFER_MEMCPY
// Dark calculation (0 - 255 for EPDiy but really only 16 levels)
// Only applied withouth FRAMEBUFFER_MEMCPY
uint8_t black_16_multi = 8; // 16 is multiplied by this. A lower number than 16 is darker

// Highly experimental: X4 size will make a pixel in 4 pixel (At the cost of loosing quality)
// This mode works only without the FRAMEBUFFER_MEMCPY defined!
//#define PIXEL_X4
// Define partial update mode
EpdDrawMode partialMode = MODE_GC16; // MODE_GC16 / MODE_GL16 with 16 grays but slower. MODE_DU faster only B/W

extern "C"
{
   void app_main();
}

void app_main(void)
{
  printf("CalEPD version: %s for Plasticlogic.com\nVIDEO demo\n", CALEPD_VERSION);
  
  uint8_t *videobuffer = (uint8_t*)malloc(FRAME_SIZE);
  #ifdef FRAMEBUFFER_MEMCPY
    uint16_t line_size = EPD_WIDTH/2;
    uint8_t *linebuffer = (uint8_t*)malloc(line_size);
  #endif
  ESP_LOGI(TAG, "Initializing SPIFFS and allocating %d bytes for each video frame\nHEAP: %d", FRAME_SIZE, xPortGetFreeHeapSize());
  

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
  uint16_t millis_render_st = 0;
  uint16_t millis_render_end = 0;

  while (fread(videobuffer, FRAME_SIZE, 1, fp)) {
    frame_nr++;
    millis_render_st = esp_timer_get_time() / 1000;
    #ifndef FRAMEBUFFER_MEMCPY
      for (uint32_t bp = 0; bp < FRAME_SIZE; ++bp) {
        #ifdef PIXEL_X4
        if (x_pix_read > video_width*2-1) {
          x_pix_read = 0;
          y_line+=2;
          if (y_line > video_height*2) break;
        }
        #else
        if (x_pix_read > video_width-1) {
          x_pix_read = 0;
          y_line++;
          if (y_line > video_height) break;
        }
        #endif
        uint8_t low_bits =  videobuffer[bp] & 0x0F;
        uint8_t high_bits = videobuffer[bp] >> 4;
      
        #ifdef PIXEL_X4
        display.drawPixel(x_pix_read, y_line, low_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+1, y_line, high_bits *black_16_multi-1);
        display.drawPixel(x_pix_read, y_line+1, low_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+1, y_line+1, high_bits *black_16_multi-1);

        display.drawPixel(x_pix_read+2, y_line, high_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+3, y_line, low_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+2, y_line+1, high_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+3, y_line+1, low_bits *black_16_multi-1);
        x_pix_read+=4;
        #else 
        display.drawPixel(x_pix_read, y_line, low_bits *black_16_multi-1);
        display.drawPixel(x_pix_read+1, y_line, high_bits *black_16_multi-1);
        x_pix_read+=2;
        #endif      
      }
      
    #else
    // Copy directly in EPD framebuffer the whole X line of the video
      for (uint32_t bp = 0; bp < FRAME_SIZE; bp += video_width/2) {
        memset(linebuffer, 255, line_size);
        memcpy(linebuffer, &videobuffer[bp], video_width/2);
        display.cpyFramebuffer(0, y_line, linebuffer, line_size);
        y_line++;
      }
    #endif

    y_line = 0;
      
    display.updateWindow(0, 0, video_width, video_height, partialMode); 
    
    millis_render_end = esp_timer_get_time() / 1000;
    printf("F%d R%d\n", frame_nr, millis_render_end-millis_render_st);
  };

  ESP_LOGI(TAG, "End of video");
  fclose(fp); 
}
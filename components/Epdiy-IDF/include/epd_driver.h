/**
 * A high-level library for drawing to an EPD.
 * Original component: https://github.com/vroland/epdiy by vroland
 * Refactored by: martinberlin (cale.es)
 */

#include "esp_attr.h"
#include <stdbool.h>
#include <stdint.h>
#include "esp_assert.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "xtensa/core-macros.h"
#include <string.h>

// FIX: This should be in the epaper class
#define EPD_WIDTH 960
#define EPD_HEIGHT 540
// number of bytes needed for one line of EPD pixel data.
#define EPD_LINE_BYTES EPD_WIDTH / 4

#define CLEAR_BYTE 0B10101010
#define DARK_BYTE 0B01010101

// minimal draw time in ms for a frame layer,
// which will allow all particles to set properly.
#ifndef MINIMUM_FRAME_TIME
#define MINIMUM_FRAME_TIME (min(k, 10))
#endif

#ifndef epd_i2s_bus_h
#define epd_i2s_bus_h
/// An area on the display.
typedef struct {
  /// Horizontal position.
  int x;
  /// Vertical position.
  int y;
  /// Area / image width, must be positive.
  int width;
  /// Area / image height, must be positive.
  int height;
} Rect_t;

/// The image drawing mode.
enum DrawMode {
  /// Draw black / grayscale image on a white display.
  BLACK_ON_WHITE = 1 << 0,
  /// "Draw with white ink" on a white display.
  WHITE_ON_WHITE = 1 << 1,
  /// Draw with white ink on a black display.
  WHITE_ON_BLACK = 1 << 2,
};

/// Font drawing flags
enum DrawFlags {
  /// Draw a background.
  ///
  /// Take the background into account
  /// when calculating the size.
  DRAW_BACKGROUND = 1 << 0,
};

/// Font properties.
typedef struct {
  /// Foreground color
  uint8_t fg_color : 4;
  /// Background color
  uint8_t bg_color : 4;
  /// Use the glyph for this codepoint for missing glyphs.
  uint32_t fallback_glyph;
  /// Additional flags, reserved for future use
  uint32_t flags;
} FontProperties;

// Heap space to use for the EPD output lookup table, which
// is calculated for each cycle.
static uint8_t *conversion_lut;
static QueueHandle_t output_queue;

typedef struct {
  const uint8_t *data_ptr;
  SemaphoreHandle_t done_smphr;
  SemaphoreHandle_t start_smphr;
  Rect_t area;
  int frame;
  enum DrawMode mode;
  const bool *drawn_lines;
} OutputParams;

static OutputParams fetch_params;
static OutputParams feed_params;


class EpdI2SDriver
{
  public:
    /** Initialize the ePaper display */
    void epd_init();

    /** Deinit the ePaper display */
    void epd_deinit();

    /** Enable display power supply. */
    void epd_poweron();

    /** Disable display power supply. */
    void epd_poweroff();

    /** Clear the whole screen by flashing it. */
    void epd_clear();

    /**
     * Clear an area by flashing it.
     *
     * @param area: The area to clear.
     */
    void epd_clear_area(Rect_t area);

    /**
     * Clear an area by flashing it.
     *
     * @param area: The area to clear.
     * @param cycles: The number of black-to-white clear cycles.
     * @param cycle_time: Length of a cycle. Default: 50 (us).
     */
    void epd_clear_area_cycles(Rect_t area, int cycles, int cycle_time);

    /**
     * Darken / lighten an area for a given time.
     *
     * @param area: The area to darken / lighten.
     * @param time: The time in us to apply voltage to each pixel.
     * @param color: 1: lighten, 0: darken.
     */
    void epd_push_pixels(Rect_t area, short time, int color);

    /**
     * Draw a picture to a given area. The image area is not cleared and assumed
     * to be white before drawing.
     *
     * @param area: The display area to draw to. `width` and `height` of the area
     *   must correspond to the image dimensions in pixels.
     * @param data: The image data, as a buffer of 4 bit wide brightness values.
     *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
     * multiple rows, images of uneven width must add a padding nibble per line.
     */
    void IRAM_ATTR epd_draw_grayscale_image(Rect_t area, const uint8_t *data);

    /**
     * Draw a picture to a given area, with some draw mode.
     * The image area is not cleared before drawing.
     * For example, this can be used for pixel-aligned clearing.
     *
     * @param area: The display area to draw to. `width` and `height` of the area
     *   must correspond to the image dimensions in pixels.
     * @param data: The image data, as a buffer of 4 bit wide brightness values.
     *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
     * multiple rows, images of uneven width must add a padding nibble per line.
     * @param mode: Configure image color and assumptions of the display state.
     */
    void IRAM_ATTR epd_draw_image(Rect_t area, const uint8_t *data,
                                  enum DrawMode mode);

    /**
     * Same as epd_draw_image, but with the option to specify
     * which lines of the image should be drawn.
     *
     * @param drawn_lines: If not NULL, an array of at least the height of the
     * image. Every line where the corresponding value in `lines` is `false` will be
     * skipped.
     * @param data: The image data, as a buffer of 4 bit wide brightness values.
     *   Pixel data is packed (two pixels per byte). A byte cannot wrap over
     * multiple rows, images of uneven width must add a padding nibble per line.
     * @param mode: Configure image color and assumptions of the display state.
     * @param drawn_lines: Optional line mask.
     *   If not NULL, only draw lines which are marked as `true`.
     */
    void IRAM_ATTR epd_draw_image_lines(Rect_t area, const uint8_t *data,
                                        enum DrawMode mode,
                                        const bool *drawn_lines);

    void IRAM_ATTR epd_draw_frame_1bit(Rect_t area, const uint8_t *ptr,
                                      enum DrawMode mode, int time);

    void IRAM_ATTR epd_draw_frame_1bit_lines(Rect_t area, const uint8_t *ptr,
                                            enum DrawMode mode, int time,
                                            const bool *drawn_lines);

    /**
     * @returns Rectancle representing the whole screen area.
     */
    Rect_t epd_full_screen();

    /**
     * Draw a picture to a given framebuffer.
     *
     * @param image_area: The area to copy to. `width` and `height` of the area
     *   must correspond to the image dimensions in pixels.
     * @param image_data: The image data, as a buffer of 4 bit wide brightness
     * values. Pixel data is packed (two pixels per byte). A byte cannot wrap over
     * multiple rows, images of uneven width must add a padding nibble per line.
     * @param framebuffer: The framebuffer object,
     *   which must be `EPD_WIDTH / 2 * EPD_HEIGHT` large.
     */
    void epd_copy_to_framebuffer(Rect_t image_area, const uint8_t *image_data,
                                uint8_t *framebuffer);

    /**
     * Draw a pixel a given framebuffer.
     *
     * @param x: Horizontal position in pixels.
     * @param y: Vertical position in pixels.
     * @param color: The gray value of the line (0-255);
     * @param framebuffer: The framebuffer to draw to,
     */
    void epd_draw_pixel(int x, int y, uint8_t color, uint8_t *framebuffer);
    
    void reorder_line_buffer(uint32_t *line_data);
    // All rest will be done by GFX

  private:

    void write_row(uint32_t output_time_dus);
    void IRAM_ATTR skip_row(uint8_t pipeline_finish_time);
    void IRAM_ATTR calc_epd_input_4bpp(
      const uint32_t *line_data,
      uint8_t *epd_input, uint8_t k,
      const uint8_t *conversion_lut);
    void IRAM_ATTR calc_epd_input_1bpp(
      const uint8_t *line_data, 
      uint8_t *epd_input,
      enum DrawMode mode);
      
    void IRAM_ATTR reset_lut(uint8_t *lut_mem, enum DrawMode mode);
    void IRAM_ATTR update_LUT(uint8_t *lut_mem, uint8_t k, enum DrawMode mode);
    void IRAM_ATTR nibble_shift_buffer_right(uint8_t *buf, uint32_t len);
    void IRAM_ATTR bit_shift_buffer_right(uint8_t *buf, uint32_t len, int shift);
    void IRAM_ATTR feed_display(OutputParams *params);
    void IRAM_ATTR provide_out(OutputParams *params);

    // status tracker for row skipping
    uint32_t skipping;
};

#endif
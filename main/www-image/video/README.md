## Video demo

This is demo that should be compiled independently from the root folder since uses files stored in SPIFFs.

So just run idf.py build in this folder.

## Encoding videos in grayscale

Here there is an [interesting documentation](https://newbedev.com/how-to-encode-grayscale-video-streams-with-ffmpeg) about doing this with ffmpeg.

To be tried:

   PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
   PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
   PIX_FMT_GRAY8A,    ///< 8bit gray, 8bit alpha

## If using epdiy try out different update and rendering methods

```c
    EpdDrawMode partialMode = MODE_DU;

/* Depending on the epaper and the waveforms available:
     MODE_DU   the fastest with 200ms per frame but "goes from any color to black for white only"
     MODE_GL16 works great but is too slow (about 2 secs per frame)
     MODE_A2 would be the best for animation but is not available in default waveform
*/
```

Try out also to copy each video Row directly into the EPD framebuffer activating FRAMEBUFFER_MEMCPY

```c
// Instead of using slow display.drawPixel write directly in RAM:
for (uint32_t bp = 0; bp < FRAME_SIZE; bp += video_width/2) {
   memset(linebuffer, 255, line_size);
   memcpy(linebuffer, &videobuffer[bp], video_width/2);
   display.cpyFramebuffer(0, y_line, linebuffer, line_size);
   y_line++;
}
```
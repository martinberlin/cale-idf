Download and render images examples
===================================

- In order to render an image using as little RAM as possible you can try one of the following examples:

```
   main/cale.cpp          it works for monochrome and 3 color epapers (B/W/Red or B/W/Yellow)
   main/cale-7-color.cpp  for the Waveshare 7 color Acep epaper
   demos/cale-sensor.cpp  same as cale.cpp but with the addition that a sensor can trigger new image download and render
   main/www-jpg-render    standalone project. Needs to be build in that folder. Needs external RAM
```

Is capable to download a compressed image and render it in your epaper. It looks the best with 16 grayscale capable parallel einks but there is also a monochrome version.

Please note that JPG decompress needs an external RAM capable ESP32 like WROVER-B or similar.
Also there are several ESP32S2 that come with PSRAM. epdiy based boards need anyways external RAM otherwise has no space to save the Epaper framebuffer.

### Additional examples: 

- **RGB4 video demo** I discovered reading about FFMPEG to convert videos to grayscale that there is a format that encodes a video in RAW rgb4 format. This is a mini-project that you need to build in the **/main/www-image/video** folder, because we are adding the spiffs_image in the CMakeFiles and it will be silly to do it for the whole project.

The RGB4 pixel format is awesome since you can directly copy to memory the video rows to the parallel epaper framebuffer (uses the same 4 bit-per-pixel format). The [results are less than satisfactory](https://twitter.com/martinfasani/status/1479886636917866508) since the MODE_A2 waveform for animation was not available in the display I tested. But anyways, adding a small delay you can make an awesome streaming offline gallery, encoding photos as slides that are read from an SD card or SPIFFs (Note that the example only implements SPIFFs so far)

## How to encode videos

Just download any from YouTube with any online web-downloader. Using linux /mac then encode only some seconds to RGB4:
 
    # Preview clip -ss=start -t=time to cut
    ffmpeg -ss 00:01:42 -t 00:00:10 -i input.mp4 -vf "fps=1" output.mp4

    # Encode that part in RGB4 pixel format
    ffmpeg -ss 00:01:42 -t 00:00:10 -i input.mp4 -vf "fps=1,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -c:v rawvideo -pix_fmt rgb4 output.rgb

**Quick links:**

- [www-jpg-render example](https://github.com/martinberlin/cale-idf/tree/master/main/www-jpg-render)

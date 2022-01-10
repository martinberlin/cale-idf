Download and render images examples
===================================

- In order to render a BMP using as little RAM as possible you can try one of the following examples:

```
   main/cale.cpp          it works for monochrome and 3 color epapers (B/W/Red or B/W/Yellow)
   main/cale-7-color.cpp  for the Waveshare 7 color Acep epaper
   demos/cale-sensor.cpp  same as cale.cpp but with the addition that a sensor can trigger new image download and render
   main/www-jpg-render    standalone project. Needs to be build in that folder
```

Is capable to download a compressed image and render it in your epaper. It looks the best with 16 grayscale capable parallel epapers but there is also a monochrome version.

Please note that JPG decompress needs an external RAM capable ESP32 like WROVER-B or similar.
Also there are several ESP32S2 that come with PSRAM.

### Additional examples: 

- **GIF example** using Larry Bank [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF/) component.
  Disclaimer: The homer works but I could never make it work completely when they are big black or gray areas. If someone knows what's the way to do it right please do a pull request.
- **RGB4 video demo** I discovered reading about FFMPEG to convert videos to grayscale that there is a format called that encodes a video in RAW rgb4 format. This is a mini-project that you need to build in the /main/www-image/video folder, because we are adding the spiffs_image in the CMakeFiles and it will be silly to do it for the whole project.

The RGB4t is awesome since you can directly memcopy the lines of the video to a parallel epaper since they use the same 4 bit-per-pixel framebuffer. The [results are less than satisfactory](https://twitter.com/martinfasani/status/1479886636917866508) since the MODE_A2 waveform for animation was not available in the display I tested. But anyways, adding a small delay you can make an awesome streaming offline gallery, encoding photos as slides that are read from an SD card or SPIFFs (Note that the example only implements SPIFFs so far)


**Quick links:**

- [www-jpg-render example](https://github.com/martinberlin/cale-idf/tree/master/main/www-jpg-render)

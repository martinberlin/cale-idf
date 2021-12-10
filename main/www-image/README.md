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

**Quick links:**

- [www-jpg-render example](https://github.com/martinberlin/cale-idf/tree/master/main/www-jpg-render)

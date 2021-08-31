Download and render image example
=================================

After discussing the idea of collaborating adding an WiFi download and render example in the epdiy.slack.com we decided to also add a JPG decoding example suggested by @vroland.
This example is now ported also to Cale-idf so you can use it on your displays. There is still a catch and it's that is designed to work using PSRAM and also 16 grayscale capable epapers. 
But of course analyzing how it's done you can play around with the colors and send the right buffer to any display. Comes in 3 versions:

 - **jpg-render.cpp**
    Takes aprox. 1.5 to 2 seconds to download a 200Kb jpeg. Uses a decoded image buffer and has optional JPG dithering (Better to the eye in grayscale transitions)
    Has auto-centering for smaller images.
  
 - **jpg-render-v2.cpp** Takes less time since does not uses a decoded_buffer. Uses less RAM and is faster but it looses dithering functionality and auto-centering.
 
 - **jpgdec-render.cpp** Is the faster since uses [@bitbank/JPEGDECODER](https://github.com/bitbank2/JPEGDEC/). Comes already with 4BIT dithering included so it outperforms jpg-render.cpp

In our opinion V2 is a good trade-off to use less RAM and still have a very good JPG rendering quality.
The benefit of using TJPG is that it's already on ESP32 ROM. But if you need more decoding speed and dithering in the comonent, JPEGDEC is an excellent choice.

To select one of the two versions just uncomment only one app_sources in the CMakeLists.txt 

```
#set(app_sources "jpg-render.cpp")
# Faster, uses less RAM, but looses dithering functionality:
set(app_sources "jpg-render-v2.cpp")
```

Detailed statistics:

```
// v1
48772 bytes read from https://loremflickr.com/960/540

 757 ms - image decompression
 297 ms - jpeg draw
 1728 ms - download
 2782 ms - total time spent

// v2
88 Kb cat image (below)
851 ms - image decompression
1461 ms - download
2312 ms - total time
```
![88Kb Cat in Lilygo EPD47](https://pbs.twimg.com/media/E934uz8WEAIyVLP?format=jpg&name=small)

In order to get around using an ESP32 without external RAM a big refactoring is needed. Since JPEG images are compressed you need a buffer to store the image and another one to decompress it, if you want to do image manipulation like JPG dither.
All that takes a big amount of RAM (Unless you use a small image)

**Note:** Statistics where taken with the 4.7" Lilygo display 960x540 pixels and may be significantly higher using bigger displays.

Building it
===========

Do not forget to update your WiFi credentials and point it to a proper URL that contains the image with the right format:

```c
// WiFi configuration in menuconfig -> CALE Configuration
// EPD class. Include the one that is right for your epaper
#include <gdeh042Z96.h>
EpdSpi io;
Gdeh042Z96 display(io);
// Update here with the width and height of your Epaper
#define EPD_WIDTH  400
#define EPD_HEIGHT 300
// Feel free to replace this for your own URL
#define IMG_URL ("https://loremflickr.com/" STR(EPD_WIDTH) "/" STR(EPD_HEIGHT))
```

Progressive JPG images are not supported.

    idf.py menuconfig
    idf.py flash monitor

If you are using a parallel epaper do forget to select in:

    Component config -> E-Paper Driver -> Select Board & Display you are using

Note that you can use a random image taken from loremflickr.com. Or you can use any URL that points to a valid Image, take care to use the right JPG format, or you can also use the image-service [cale.es](https://cale.es) to create your own gallery. Otherwise expect a lot of cats.
A good thing for those rendering JPG in monochrome displays is that CALE offers dithering, so you can select a 2x2 or 3x3 dithering, simulating grays.

If your epaper resolution is not listed, just send me an email, you can find it on my [profile page on Github](https://github.com/martinberlin).

Using HTTPS
===========

Using SSL requires a bit more effort if you need to verify the certificate. For example, getting the SSL cert from loremflickr.com needs to be extracted using this command:

    openssl s_client -showcerts -connect www.loremflickr.com:443 </dev/null

The CA root cert is the last cert given in the chain of certs.
To embed it in the app binary, the PEM file is named in the component.mk COMPONENT_EMBED_TXTFILES variable. This is already done for this random picture as an example.

**Important note about secure https**
Https is proved to work on stable ESP-IDF v4.2 branch. Using latest master I've always had resets and panic restarts, only working randomly. Maybe it's an issue will be fixed.

Also needs the main Stack to be bigger otherwise the embedTLS validation fails:
Just 1Kb makes it work: 
CONFIG_ESP_MAIN_TASK_STACK_SIZE=4584

You can set this in **idf.py menuconfig**

     -> Component config -> ESP32-specific -> Main task stack size

Please be aware that in order to validate SSL certificates the ESP32 needs to be aware of the time. Setting the define VALIDATE_SSL_CERTIFICATE to true will make an additional SNTP server ping to do that. That takes between 1 or 2 seconds more.

Setting VALIDATE_SSL_CERTIFICATE to false also works skipping the .cert_pem in the esp_http_client_config_t struct. 

### Pull requests accepted

If you find a way to optimize this or make a better version go ahead and make a pull request describing what is your goal and we will code-review it and merge it.

### Thanks

For all the support, ideas and conversation, I would like to thank:

- @vroland and Team, creators of the amazing [ESP32 EPDiy parallel epaper controller](https://github.com/vroland/epdiy/)
- Larry creator of [@bitbank/JPEGDECODER](https://github.com/bitbank2/JPEGDEC/) among many other interesting libraries
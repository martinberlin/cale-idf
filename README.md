# CALE ESP-IDF beta

This is the beginning, and a very raw try, to make CALE compile in the Espressif IOT Development Framework. But it's also the development area of our professional [epaper component CalEPD](https://github.com/martinberlin/CalEPD). 

**Epaper ESP-IDF component with GFX capabilities and multi SPI support**

## Branches

**master**...    -> stable version
    v.0.9.5 [PlasticLogic Added new EPD manufacturer](https://plasticlogic.com)
    v.0.9.2 Wave12I48   Added Waveshare 12.48" multi epaper display
    v.0.9.1 Gdew075T7   Added Waveshare/Good display 7.5" V2 800*480
    v.0.9   Gdew0213i5f First testeable version with a 2.13" b/w epaper display Gdew0213i5f
    
    

**refactor/oop** -> Making the components base, most actual branch, where new models are added. Only after successfull testing they will be merged in master

**refactor/XXX** -> Some other branch where I refactor things for a manufacturer. Do not use inestable branches!

tft_test         -> Original SPI master example from ESP-IDF 4 just refactored as a C++ class. Will be kept for historic reasons


The aim is to learn good how to code and link classes as git submodules in order to program the epaper display driver the same way. The goal is to have a tiny "human readable" code in cale.cpp main file and that the rest is encapsulated in classes.

### Epaper demos

Open the /main/CMakeLists.txt file to select what is the C++ file that will be compiled as your main program. Just uncomment one of the SRCS lines:

    idf_component_register(
      # Main CALE 
      #SRCS "cale.cpp" -> CALE Firmware for IDF
       SRCS "demo-fonts.cpp"
      #SRCS "demo-epaper.cpp"
      INCLUDE_DIRS ".")

This configuration will just compile the Fonts demo to test your epaper display. Use cale.cpp if you are looking forward to compile our [CALE Firmware](https://github.com/martinberlin/eink-calendar). This is the ESP-IDF version of that Eink-calendar esp32-arduino Firmware. In CALE.es there is a web-service that can help you making dynamic Screens for your epapers.

## Fonts support and German characters

Please check [Adafruit page on adding new fonts](https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts#adding-new-fonts-2002831-18) as a start. In order to add the whole character range, you need to set from -> to ranges after calling font convert. Just go to the /components/Adafruit-GFX/fontconvert directory and run:

./fontconvert /usr/share/fonts/truetype/ubuntu/YOURTTF.ttf 18 32 252 > ../Fonts/YOURTTFNAME.h

Be aware that PROGMEM is not supported here since we are not in the Arduino framework. So just remove it from the generated fonts.

As an example with all characters including German umlauts ( ä ö ü and others like ß) I left Ubuntu_M18pt8b ready in the Fonts directory. Be aware that using the whole character spectrum will also take part of your programs memory.

### Submodules

Not being used at the moment since all test and development happens here. Only when there are new working models they will be pushed as new release in the component repository:
[CalEPD epaper component](https://github.com/martinberlin/CalEPD) is published on version 0.9

ESP-IDF uses relative locations as its submodules URLs (.gitmodules). So they link to GitHub. To update the submodules once you **git clone** this repository:

    git submodule update --init --recursive
    
to download the submodules (components) for this project.
Reminder for myself, in case you update the module library just execute:

    # pull all changes for the submodules
    git submodule update --remote

### Compile this 

If it's an ESP32

    idf.py -D IDF_TARGET=esp32 menuconfig

If it's an ESP32S2

    idf.py -D IDF_TARGET=esp32s2 menuconfig

Make sure to edit **Display configuration** in the Kconfig menuoptions.
**CALE configuration** has for the moment only a LEDBUILTIN_GPIO setting so it's not important at all. Later will be the place to configure CALE.es image url.

And then just build and flash

    idf.py build
    idf.py flash

To clean and start again in case you change target (But usually no need to run)

    idf.py fullclean

To open the serial monitor

    idf.py monitor

Please note that to exit the monitor in Espressif documentation says Ctrl+] but in Linux this key combination is:

    Ctrl+5

## config-examples/

In the config-examples folder we left samples of GPIO configurations. For example:

- Wave12I48 has the GPIOs ready to use w/Waveshare socket for ESP32-devKitC
- S2 has a sample GPIO config to be used after idf.py set-target esp32s2 (Only for S2 chips)

## SPI speed

If you instantiate display.init(true) it activates verbose debug and also lowers SPI frequency to 50000. Most epapers accept a frequency up to 4 Mhz. 
We did this on debug to be able to sniff with an ESP32 SPI slave "man in the middle" what commands are sent to the display so we can detect mistakes. Even if you print it, is not the same as to hear on the line, this is the real way to reverse engineer something. Hear what the master is saying in a library that works.

    +    uint16_t multiplier = 1000;
    +    if (debug_enabled) {
    +        frequency = 50;
    +        multiplier = 1;
    +    }

Due to restrictions in C++ that I'm not so aware about there is a limitation when using big integers in the structs { }
So SPI frequency is calculated like:

    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*multiplier*1000,  // --> Like this being the default 4 Mhz
        .input_delay_ns=0,
        .spics_io_num=CONFIG_EINK_SPI_CS,
        .flags = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
        .queue_size=5
    };

Feel free to play with Espressif IDF SPI settings if you know what you are doing ;)

## Multi-SPI displays

A new breed of supported displays is coming being the first the [Wave12I48 12.48" b/w epaper from Waveshare](https://github.com/martinberlin/cale-idf/wiki/Model-wave12i48.h).
This is the first component that support this multi epaper displays abstracting their complexity so you can treat it as a normal 1304x984 single display and use all the Adafruit GFX methods and fonts to render graphics over it.

## Watchdogs feeding for large buffers

In Buffers for big displays like 800*480 where the size is about 48000 bytes long is necessary to feed the watchdog timer and also make a small delay. I'm doing it this way:

    +    // Let CPU breath. Withouth delay watchdog will jump in your neck
    +    if (i%8==0) {
    +       rtc_wdt_feed();
    +       vTaskDelay(pdMS_TO_TICKS(1));
    +     }

Again, if you know more about this than me, feel free to suggest a faster way. It's possible to disable also the [watchdogs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html) but of course that is not a good practice to do so.

## References

https://github.com/krzychb/esp-epaper-29-ws (2 Years ago, probably ESP-IDF 3.0)

[esp32.com "epaper" search](https://esp32.com/search.php?keywords=epaper&fid%5B0%5D=13)

[GxEPD Epaper library](https://CALE.es) GxEPD is to use with Espressif Arduino Framework. 

[CALE.es Web-service](https://CALE.es) a Web-Service that prepares BMP & JPG Screens with the right size for your displays

[CALE.es Arduino-espressif32 firmware](https://github.com/martinberlin/eink-calendar)

### Credits 

GxEPD has been a great resource to start with. For CalEPD component, we mantain same Constants only without the **Gx prefix** and use the same driver nomenclature as GxEPD library, just in small case.
Hats off to Jean-Marc Zingg that was the first one to make such a great resource supporting so many Eink displays.

Thanks to all the developers interested to test this like @IoTPanic and others that pushed me to improve my C++ skills.
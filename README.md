![CALE Logo](/config-examples/assets/cale-idf.svg)

### Requirements

* esp32 or esp32S2
* Espressif IDF framework 4.3
* An SPI epaper (see wiki for supported models)

Cale-idf is the official ESP-IDF firmware of our Web-Service [CALE.es](https://cale.es) and also the repository where the development of CalEPD epaper component takes place. The main class extends Adafruit GFX so this library has full geometric functions and also fonts including German/Spanish/French special characters support.
On latest release, CalEPD has also support for FocalTech I2C touch panel, enabling you to make simple UX interfaces using small epaper displays. This is optional and can be enabled only when the Firmware requires touch.
Please check the [Wiki](https://github.com/martinberlin/cale-idf/wiki) for latest news and to see what displays are supported. The Wiki is the perfect place to make updates that are not branch dependant so our documentation efforts will be focused there.
CalEPD supports currently the most popular epaper sizes and four color models (4.2, 5.83 and 7.5 inches).

- Use **refactor/oop** to try the latest features. Only after days or even weeks of testing, it will be merged in master, and eventually land in a new [CalEPD epaper component release](https://github.com/martinberlin/CalEPD)

## Requesting for new models

If your epaper model is not there just open an Issue and send us one epaper with the SPI interface. If we can make a working implementation and new C++ class then you can use it in your Firmware and we keep the eink as a payment for our effort. If we fail and cannot make a working implementation then it comes back to you at no cost.

## CALE Firmware

**CALE does only 3 things at the moment and is very easy to set up:**

1. It connects to cale.es and downloads a Screen bitmap.
2. In "Streaming mode" it pushes the pixels to Adafruit GFX buffer and at the end renders it in your Epaper.
3. It goes to sleep the amount of minutes you define in the ESP-IDF menuconfig

And of course wakes up after this deepsleep and goes back to point 1 making it an ideal Firmware if you want to refresh an Events calendar or weather Forecast display. It does not need to be tied to our CALE service. You can use your own full url to your bitmap image. We just recommend to use CALE.es since you can easily connect it to external APIs and have a living epaper.

    RELEASES
    v.0.9.6 First color epaper: Gdew0583z21 B/W/RED
    v.0.9.5 [PlasticLogic Added new EPD manufacturer](https://plasticlogic.com)
    v.0.9.2 Wave12I48   Added Waveshare 12.48" multi epaper display
    v.0.9.1 Gdew075T7   Added Waveshare/Good display 7.5" V2 800*480
    v.0.9   Gdew0213i5f First testeable version with a 2.13" b/w epaper display Gdew0213i5f

    ROADMAP
    Rest of 2020 Enabling touch support to enable UX design in ESP32
    2020-Sep Optimizing instantiation and configuration
    2020-Aug Adding color epapers 5.83 and 7.5 inches
    2020-Jul Added PlasticLogic as a new brand with 4 wire SPI (uses MISO)
    
## News

- Multi SPI epaper 12.48 class Wave12I48 is working. This epaper has Waveshare added electronics and ESP32 support. It has a 160 Kb buffer, so it leaves no DRAM for your program. Check my PSIRAM hack to replace the DevKitC with a ESP32 WROVER-B board if you want to have a working sketch with additional libraries (WiFi, download image from www, etc) Without PSIRAM only a very basic sketch can be made.

[News section has been moved to the Wiki section](https://github.com/martinberlin/cale-idf/wiki/CalEPD-news)

**CALE-IDF uses this components:**

- [CalEPD](https://github.com/martinberlin/CalEPD) the epaper component
- [Adafruit GFX for ESP-IDF](https://github.com/martinberlin/Adafruit-GFX-Library-ESP-IDF) My own fork of Adafruit display library

They are at the moment included without git submodules so we can develop fast without updating them all the time. But they are also available to be used as your project ESP-IDF components.

## Configuration

Make sure to set the GPIOs that are connected from the Epaper to your ESP32. Data in in your epaper (DIN) should be connected to MOSI:

![CALE config](/config-examples/assets/menuconfig-display.png)

And then set the image configuration and deepsleep minutes. Here you can also set the rotation for your Eink display:

![Display config](/config-examples/assets/menuconfig-cale.png)

Optionally if you use touch, for example with 2.7 inches gdew027w3-T epaper, you should configure also FT6X36 Gpios:

![Optional Touch panel](/config-examples/assets/menuconfig-touch.png)

Needs 3.3v, a common GND, SDA, SCL for I2C communication, and a input INT pin that signalizes on Low that there is touch data ready.

## CalEPD component

[CalEPD is an ESP-IDF component](https://github.com/martinberlin/CalEPD) to drive epaper displays with ESP32 / ESP32S2 and it's what is sending the graphics buffer to your epaper behind the scenes. It's designed to be a light C++ component to have a small memory footprint and run as fast as posible, leaving as much memory as possible for your program. Note that the pixels buffer, takes 1 byte to store 8 pixels on each color, so depending on your epaper size may need external PSRAM. Up to 800 * 480 pixels on a monochrome eink it runs stable and there is still free DRAM for more.

## Branches

**master**...    -> stable version (ChangeLog moved to Wiki)

**refactor/oop** -> Making the components base, most actual branch, where new models are added. Only after successfull testing they will be merged in master. Inestable branch do not use on Firmware that you ship to a client.

tft_test         -> Original SPI master example from ESP-IDF 4 just refactored as a C++ class. Will be kept for historic reasons

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

If it's an ESP32:

    idf.py set-target esp32

If it's an ESP32S2:

    idf.py set-target esp32s2

Make sure to edit **Display configuration** in the Kconfig menuoptions:

    idf.py menuconfig

**CALE configuration** is the section to set the bitmap URL (non-ssl for the moment), deepsleep until next refresh, and optional display rotation

And then just build and flash:

    idf.py build
    idf.py flash monitor

To clean and start again in case you change target (But usually no need to run)

    idf.py fullclean

To open the serial monitor only

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
Please note that this big display needs a 160 Kb buffer leaving no DRAM available for anything more on your ESP32. So you can either make a very simple program that renders sensor information, or do everything you want, but adding PSRAM for the GFX buffer. Think about ESP32-WROOVER as a good candidate. 

## Watchdogs feeding for large buffers

In Buffers for big displays like 800*480 where the size is about 48000 bytes long is necessary to feed the watchdog timer and also make a small delay. I'm doing it this way:

    +    // Let CPU breath. Withouth delay watchdog will jump in your neck
    +    if (i%8==0) {
    +       rtc_wdt_feed();
    +       vTaskDelay(pdMS_TO_TICKS(1));
    +     }

Again, if you know more about this than me, feel free to suggest a faster way. It's possible to disable also the [watchdogs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html) but of course that is not a good practice to do so.

### References and related projects

[CALE.es Web-service](https://CALE.es) a Web-Service that prepares BMP & JPG Screens with the right size for your displays

[CALE.es Arduino-espressif32 firmware](https://github.com/martinberlin/eink-calendar)


[GxEPD Epaper library](https://CALE.es) GxEPD is to use with Espressif Arduino Framework. 

Searches that we do to check how other components evolve and what is the position of this one:

["epaper" esp32.com search](https://esp32.com/search.php?keywords=epaper&fid%5B0%5D=13)

["ESP-IDF epaper" Google search](https://www.google.com/search?q=esp-idf+epaper+component)

### History

This is the beginning, and a very raw try, to make CALE compile in the Espressif IOT Development Framework. At the moment to explore how difficult it can be to pass an existing ESP32 Arduino framework project to a ESP-IDF based one and to measure how far we can go compiling this with Espressif's own dev framework. 
UPDATE: Saved for historical reasons. After starting this project I heavily adopted ESP-IDF as an IoT framework and toolsuit to build firmares. This become also the start of [CalEPD that is our own IDF component to control Epapers](https://github.com/martinberlin/CalEPD) with ESP32 / ESP32S2.

### Credits 

GxEPD has been a great resource to start with. For CalEPD component, we mantain same Constants only without the **Gx prefix** and use the same driver nomenclature as GxEPD library, just in small case.
[Strange-v](https://github.com/strange-v/FT6X36) for the creation of the FocalTech touch library, that I forked to make the FT6X36-IDF component.
Hats off to Jean-Marc Zingg that was the first one to make such a great resource supporting so many Eink displays. Please note that there are no plans to port this to Arduino-framework. This repository was specially made with the purpouse to explore Espressif's own IoT development framework.

Thanks to all the developers interested to test this. Special mentions for @IoTPanic, Spectre and others that pushed me to improve my C++ skills.

### Sponsoring

If you like this component and it made your life easier please consider becoming a sponsor where you can donate as little as 2 u$ per month. Just click on:
❤ Sponsor  on the top right

♢ For cryptocurrency users is also possible to help this project transferring Ethereum:

    0x65B7EF685E5B493603740310A84268c6D59f58B5

We are thankful for the support and contributions so far!

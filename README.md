![FastEPD-logo-github](https://github.com/user-attachments/assets/85e74ce2-5c6f-4cc5-9555-7f5d343b8172)![CALE Logo](/config-examples/assets/cale-idf.svg)

### Requirements

* esp32 or S2 / S3 / C3 MCU versions in branch [develop](https://github.com/martinberlin/cale-idf/tree/develop)
* Espressif IDF framework >= 4.2 (4.3 -> 4.4 ideally to support latest S3)
* An epaper display (see [Wiki](https://github.com/martinberlin/cale-idf/wiki) for supported models)
* If you want to have a Web-Service to deliver the image we built our own one. Just [head to CALE.es](https://cale.es) and make an account.
  This [video of UsefulElectronics](https://www.youtube.com/watch?v=7Sal9Ii7H2U) will help you to get started.

ESP32C3 /S3 also works as a target. Please check also config-examples/C3-riscv-spi where is a PIN configuration that is prove to be working. Then just select one of the SPI examples, and do a:
 **idf.py set-target esp32c3**

 **idf.py --preview set-target esp32s3**  (Only v4.4 since tried this only with beta3)

Cale-idf is the official ESP-IDF firmware of our Web-Service [CALE.es](https://cale.es) and also the repository where the development of [CalEPD](https://github.com/martinberlin/CalEPD) epaper component takes place. The main class extends Adafruit GFX so this library has full geometric functions and also fonts including German/Spanish/French special characters support.

### VSCODE and Platformio

In the repository [cale-platformio](https://github.com/martinberlin/cale-platformio) you can have a quick start skeleton to use CalEPD and Adafruit-GFX components, along with optional FocalTech touch I2C. Please be aware that there are some corrections to do by hand until we figure out what is the best way to do it. Read those in the WiKi and please give a **★ to the cale-platformio** repository if you find it useful

## News

- **We are working in a interesting new PCB design to make a smart switch using this component. If you are interested please check our [repository Bistable-smart-Switch](https://github.com/martinberlin/bistable-smart-switch) and don't be shy, give it a ★ if you like it.**
- A full pfleged version that supports WiFi provisioning using ESP-Rainmaker app is updated on the branch [feature/50-idf-v5-rainmaker](https://github.com/martinberlin/cale-idf/tree/feature/50-idf-v5-rainmaker) **Note:** It needs an external submodule component so don't forget to run:

    git submodule update --init --recursive

[For more news and announcements please check the Wiki section](https://github.com/martinberlin/cale-idf/wiki/CalEPD-news)

### Excluding components / parallel epapers

Please note that parallel driver epdiy is not anymore a requirement and after last update **epdiy V6** is not part of this repository, only linked as a git submodule. So in case you want to use our experimental implementation in C++, please pull the git submodules:

    git submodule update --init --recursive

Also please notice that if you need to exclude any of the components, like for example epdiy or any other, the fastest and most straigh-forward way is to open the CMakeLists of that component and add as the first line:

return()

That will make this component not to get in the build process.
If you are not using EPDiy to drive your epapers, this step is not needed. If you are, please go to:
CalEPD/CMakeLists.txt

And enable epdiy in the REQUIRE section and the related classes:

    # Uncomment for parallel epapers:
    "epdParallel.cpp"
    "models/parallel/ED047TC1.cpp"
    "models/parallel/ED047TC1touch.cpp"
    "models/parallel/ED060SC4.cpp"
    # Add more if you need to copying one of the existing, since not all eink sizes are supported

### Additional features

CalEPD has also support for FocalTech and L58 I2C touch panels used in Lilygo parallel epaper [EPD047](https://github.com/martinberlin/cale-idf/tree/master/components/CalEPD/models/parallel), enabling you to make simple UX interfaces using small epaper displays. This is optional and can be enabled only when the Firmware requires touch.
Please check the [Wiki](https://github.com/martinberlin/cale-idf/wiki) for latest news and to see what displays are supported. The Wiki is the perfect place to make updates that are not branch dependant so our documentation efforts will be focused there.
CalEPD supports currently the most popular epaper sizes and four color models (4.2, 5.83, 7.5 and 12.48 inches).

- Use **develop** to try the latest features. Only after days or even weeks of testing, it will be merged in master, and eventually land in a new [CalEPD epaper component release](https://github.com/martinberlin/CalEPD)
- If you are interested in LVGL / UX please check our project [lv_port_esp32-epaper](https://github.com/martinberlin/lv_port_esp32-epaper). In this experimental LVGL esp32 fork we are exploring the possibility to make UX in fast paralell displays.

Parallel epapers need to have an [EPDiy board](https://github.com/vroland/epdiy/tree/master/hardware) or a [Lilygo T5-4.7 inches epaper](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47).

## Fork policy

**Please do not Fork this repository to bookmark it**. For that use the ★ Star button. Acceptable forks fall in this three categories:

1. You found a bug and want to suggest a merge request. Then Fork it!
2. You will contribute adding a new epaper model that does not exist or add a new functionality to an existing one.
3. You will use Cale-idf as a base to create something new. But in that case it would be better to fork the components. 

This advice is because we don't like having copies of the whole repository without any reason. But it does not interfere in any way with the [Apache License](https://github.com/martinberlin/cale-idf/blob/master/LICENSE#L89) that clearly states that you might reproduce and distribute a copy of this component provided you agree with the terms mentioned there.

## Requesting for new models

If your epaper model is not there just open an Issue and send us one epaper with the SPI interface. If we can make a working implementation and new C++ class then you can use it in your Firmware and we keep the eink as a payment for our effort. If we fail and cannot make a working implementation then it comes back to you at no cost.
Also following existing classes you can do it yourself. Just check on the pull requests to see how other developers did to add their epapers!

## CALE Firmware

**CALE does only 3 things at the moment and is very easy to set up:**

1. It connects to [cale.es](http://cale.es) and downloads a Screen bitmap.
2. In "Streaming mode" it pushes the pixels to Adafruit GFX buffer and at the end renders it in your Epaper.
3. It goes to sleep the amount of minutes you define in the ESP-IDF menuconfig

It wakes up after this deepsleep and goes back to point 1 making it an ideal Firmware if you want to refresh an Events calendar or weather Forecast display. It does not need to be tied to our CALE service. You can use your own full url to your bitmap image. We just recommend to use CALE.es since you can easily connect it to external APIs and have a living epaper. Optionally you can do the same, but with a JPG, using our [www-jpg-render example](https://github.com/martinberlin/cale-idf/tree/master/main/www-jpg-render). Please note that in many cases you will require an ESP32-Wrover or similar with PSRAM.

**Different cpp examples:**

- **cale.cpp** Main example to use with monochrome or 3 color epapers from Goodisplay/Waveshare
- **cale-grayscale.cpp** Example only to use with PlasticLogic epapers, serves as footprint to do it with other models
- **cale-sensor.cpp** Same as cale.cpp but it has a sensor interrupt when a GPIO goes HIGH (Rising) this triggers a new image request
- **cale-7-color.cpp** Example to retrieve an 4 bits image and send it with up to 7 colors the 5.65 Acep epaper

Best settings on CALE.es website that we found to display color photos with cale-7-color is to use **Dither mode: 3x3** and **Bits per pixel: 24**. This is downgraded to 4bpp using dithering but that's really ok since 16 colors are more than the epaper supports. It's not a great photo quality but this epapers where designed to make labels and supermarket prices, not to display quality color pictures.

ROADMAP
    
    2023.Still adding some Goodisplay epapers. Introduction of setMonoMode (to add 4 gray mode in certain models)
    2022.Performance optimization and research in parallel eink drivers
    2021.Oct→Dec Testing other projects and small pause (Lot's of other work that are not electronics related...)
    2021.Aug→Oct Imaging libraries: Adding JPG support and optimizing processes
    2021.Jun→Aug Parallel interaction research: UX on epaper displays
    2021.Mar till June Enabling touch support to enable UX design in ESP32
    2020.Sep Optimizing instantiation and configuration
    2020.Aug Adding color epapers 5.83 and 7.5 inches
    2020.Jul Added PlasticLogic as a new brand with 4 wire SPI (uses MISO)

**CALE-IDF uses this components:**

- [CalEPD](https://github.com/martinberlin/CalEPD) the epaper component
- [Adafruit GFX for ESP-IDF](https://github.com/martinberlin/Adafruit-GFX-Library-ESP-IDF) My own fork of Adafruit display library
- [EPDiy](https://github.com/martinberlin/epdiy-rotation) it's our own fork of the parallel epaper component EPDiy with only the directory structure to use it as an IDF component

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

If you want to drive parallel Eink for digital signage or any custom project we highly recommend you trying [FastEPD](https://github.com/bitbank2/FastEPD)

<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

<svg
   width="168mm"
   height="40mm"
   viewBox="0 0 168 40"
   version="1.1"
   id="svg5"
   inkscape:version="1.1.2 (0a00cf5339, 2022-02-04)"
   sodipodi:docname="FastEPD-logo-github.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:document-units="mm"
     showgrid="false"
     inkscape:zoom="1.240133"
     inkscape:cx="378.99161"
     inkscape:cy="307.62829"
     inkscape:window-width="1862"
     inkscape:window-height="1016"
     inkscape:window-x="58"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1" />
  <defs
     id="defs2" />
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       d="M 15.0709,15.488214 2.4337857,14.147011 H 15.309336 Z M 15.577576,12.62698 2.9404624,11.285778 H 15.816012 Z M 16.084253,9.7657468 3.4471392,8.4245438 H 16.322689 Z M 16.59093,6.9045138 3.9538159,5.5633108 H 16.799561 Z M 17.067802,4.0432799 4.4604927,2.7020768 H 17.306238 Z"
       style="font-weight:bold;font-size:50.8px;line-height:1.25;font-family:'Faster One';-inkscape-font-specification:'Faster One Bold';letter-spacing:0px;word-spacing:0px;stroke-width:0.155232"
       id="path83072"
       sodipodi:nodetypes="cccccccccccccccccccc" />
    <g
       aria-label="FastEPD"
       id="text18386"
       style="font-weight:bold;font-size:35.2778px;font-family:NTR;-inkscape-font-specification:'NTR Bold';letter-spacing:0px;word-spacing:0px;stroke-width:0.264583"
       inkscape:export-filename="/home/martin/Documents/Design/FastEPD/FastEPD.png"
       inkscape:export-xdpi="102.918"
       inkscape:export-ydpi="102.918"
       transform="matrix(0.58670305,0,0,0.58670305,1.0572971,-2.5176554)">
      <path
         d="m 25.935098,32.899277 5.30545,-24.9769593 h 17.07046 L 47.50141,11.711925 H 34.565068 l -1.395265,6.614588 h 10.352519 l -0.809598,3.789608 H 32.360205 l -2.29099,10.783156 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58729" />
      <path
         d="m 63.366086,25.285611 q -0.964627,4.564754 -0.964627,5.787764 0,0.8785 0.327284,1.825902 h -4.099666 q -0.0689,-0.413412 -0.0689,-0.947402 0,-0.465088 0.03445,-0.706245 -1.533069,1.102431 -3.031686,1.498617 -1.481392,0.378961 -3.479549,0.378961 -1.481392,0 -2.566598,-0.34451 -1.06798,-0.361735 -1.636421,-0.964627 -0.568442,-0.602892 -0.826824,-1.274686 -0.241157,-0.68902 -0.241157,-1.464167 0,-1.550294 0.654569,-2.773304 0.654568,-1.240235 1.843127,-2.067058 1.188559,-0.826824 2.704402,-1.343588 1.515843,-0.516765 3.35897,-0.740696 2.945559,-0.34451 5.115971,-1.067981 0.08613,-0.447862 0.08613,-0.999078 0,-1.240235 -0.757922,-1.808676 -0.740696,-0.568442 -2.377117,-0.568442 -1.636421,0 -2.876657,0.775147 -1.240235,0.775147 -1.808676,1.860353 l -3.565676,-0.516764 q 1.223009,-2.842206 3.514,-4.20302 2.308215,-1.360813 5.925568,-1.360813 3.083362,0 4.513078,0.912951 1.446941,0.895725 1.446941,3.255617 0,1.154108 -0.327284,2.669951 z m -12.609058,2.721627 q 0,0.465088 0.275608,0.809598 0.292833,0.34451 0.740696,0.53399 0.465088,0.172255 0.930177,0.258382 0.482313,0.0689 0.981852,0.0689 2.480471,0 3.961863,-1.050755 1.481392,-1.050754 1.929255,-3.324519 l 0.155029,-0.775147 q -1.016304,0.34451 -2.446019,0.585667 -1.41249,0.223931 -2.463245,0.361735 -1.050755,0.137804 -2.032608,0.396186 -0.981853,0.258383 -1.515843,0.775147 -0.516765,0.516765 -0.516765,1.360814 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58731" />
      <path
         d="m 73.081261,33.140434 q -1.291912,0 -2.342666,-0.223932 -1.050755,-0.206706 -1.757,-0.602892 -0.68902,-0.396186 -1.188559,-0.895725 -0.499539,-0.49954 -0.775147,-1.102432 -0.258382,-0.620117 -0.378961,-1.188558 -0.103353,-0.585667 -0.103353,-1.22301 0,-0.654569 0.0689,-1.119657 h 3.789608 q -0.01723,0.120579 -0.01723,0.34451 0,1.240235 0.895726,1.912029 0.895725,0.654569 2.532147,0.654569 1.94648,0 3.01446,-0.723471 1.067981,-0.740696 1.067981,-1.843127 0,-0.844049 -0.706245,-1.188559 -0.68902,-0.361735 -2.325442,-0.671794 -1.274686,-0.241157 -2.084284,-0.430637 -0.809598,-0.206706 -1.705323,-0.551216 -0.895726,-0.361735 -1.395265,-0.775147 -0.499539,-0.430637 -0.826823,-1.085206 -0.327284,-0.671794 -0.327284,-1.533068 0,-2.032608 1.136882,-3.565676 1.136882,-1.550294 2.928333,-2.308216 1.808676,-0.757921 3.944637,-0.757921 1.464166,0 2.566598,0.275607 1.119657,0.258383 1.791451,0.671795 0.671794,0.413411 1.085205,1.016303 0.430638,0.585667 0.585667,1.171334 0.155029,0.568441 0.155029,1.223009 0,0.568441 -0.120578,1.22301 h -3.789608 q 0.01723,-0.120578 0.01723,-0.327284 0,-1.808677 -2.807755,-1.808677 -1.825902,0 -2.721627,0.637344 -0.8785,0.637343 -0.8785,1.60197 0,0.826823 0.740696,1.205784 0.740696,0.378961 2.325441,0.68902 3.445098,0.654568 4.892039,1.567519 1.464167,0.912951 1.464167,2.652726 0,2.222088 -1.291912,3.875735 -1.274686,1.636421 -3.238392,2.428794 -1.94648,0.775147 -4.220245,0.775147 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58733" />
      <path
         d="m 91.667564,29.540306 -0.258383,3.32452 q -1.291911,0.258382 -2.601048,0.258382 -2.084285,0 -2.911108,-0.671794 -0.826824,-0.68902 -0.826824,-1.998157 0,-0.775147 0.447863,-2.911107 l 2.015382,-9.491245 h -2.204862 l 0.740696,-3.445098 h 2.204862 l 1.360814,-6.390656 h 3.789608 l -1.360814,6.390656 h 2.256539 l -0.740696,3.445098 h -2.256539 l -2.067059,9.732401 q -0.0689,0.292834 -0.0689,0.620118 0,1.274686 1.533069,1.274686 l 0.103353,-0.01722 q 0.103353,-0.01723 0.327284,-0.05168 0.241157,-0.03445 0.516765,-0.0689 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58735" />
      <path
         d="M 94.544221,32.899277 99.849672,7.9223177 h 18.259018 l -0.8096,3.7896073 h -14.1249 l -1.39526,6.614588 h 10.66257 l -0.80959,3.789608 h -10.66258 l -1.481394,6.993548 h 14.124904 l -0.8096,3.789608 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58737" />
      <path
         d="m 129.0641,22.908493 h -5.68441 l -2.11873,9.990784 h -4.13412 l 5.30545,-24.9769593 h 9.5257 q 2.30821,0 3.96186,0.3617352 1.65365,0.3617353 2.61827,1.0679803 0.96463,0.7062448 1.41249,1.6536468 0.44787,0.947402 0.44787,2.187637 0,4.426951 -2.98001,7.079676 -2.96279,2.6355 -8.35437,2.6355 z m 7.04523,-8.457715 q 0,-1.533069 -1.37804,-2.135961 -1.36081,-0.602892 -3.99631,-0.602892 h -4.97817 l -1.56752,7.406961 h 5.04707 q 6.87297,0 6.87297,-4.668108 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58739" />
      <path
         d="m 149.94139,32.899277 h -9.11228 l 5.30545,-24.9769593 h 8.69887 q 2.79053,0 4.77146,0.6028921 1.99816,0.5856666 3.13504,1.7225492 1.13688,1.119656 1.63642,2.549372 0.51677,1.429716 0.51677,3.290068 0,2.44602 -0.53399,4.805912 -0.51677,2.342666 -1.6881,4.547529 -1.17133,2.187637 -2.87666,3.841284 -1.70532,1.636421 -4.25469,2.6355 -2.53215,0.981853 -5.59829,0.981853 z m 0.43064,-3.789608 q 1.757,0 3.27284,-0.499539 1.51585,-0.516765 2.60105,-1.395265 1.08521,-0.895725 1.92926,-2.032607 0.84405,-1.136883 1.34358,-2.428794 0.51677,-1.309137 0.77515,-2.583824 0.25838,-1.291911 0.25838,-2.532147 0,-2.876656 -1.68809,-4.392499 -1.6881,-1.533069 -4.89204,-1.533069 h -4.51308 l -3.68626,17.397744 z"
         style="-inkscape-font-specification:'Loma Bold Oblique'"
         id="path58741" />
    </g>
    <path
       d="M 11.958175,30.142273 -0.67893858,28.80107 H 12.196611 Z M 12.464851,27.281039 -0.17226188,25.939837 H 12.703287 Z M 12.971528,24.419806 0.33441492,23.078602 H 13.209964 Z M 13.478205,21.558572 0.84109162,20.217369 H 13.686836 Z M 13.955077,18.697338 1.3477684,17.356135 H 14.193513 Z"
       style="font-weight:bold;font-size:50.8px;line-height:1.25;font-family:'Faster One';-inkscape-font-specification:'Faster One Bold';letter-spacing:0px;word-spacing:0px;stroke-width:0.155232;fill:#ffffff"
       id="path83072-3"
       sodipodi:nodetypes="cccccccccccccccccccc" />
    <g
       aria-label="FastEPD"
       id="text18386-6"
       style="font-weight:bold;font-size:35.2778px;font-family:NTR;-inkscape-font-specification:'NTR Bold';letter-spacing:0px;word-spacing:0px;stroke-width:0.264583;fill:#ffffff"
       inkscape:export-filename="/home/martin/Documents/Design/FastEPD/FastEPD.png"
       inkscape:export-xdpi="102.918"
       inkscape:export-ydpi="102.918"
       transform="matrix(0.58670305,0,0,0.58670305,-2.0554272,12.136403)">
      <path
         d="m 25.935098,32.899277 5.30545,-24.9769593 h 17.07046 L 47.50141,11.711925 H 34.565068 l -1.395265,6.614588 h 10.352519 l -0.809598,3.789608 H 32.360205 l -2.29099,10.783156 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58729-7" />
      <path
         d="m 63.366086,25.285611 q -0.964627,4.564754 -0.964627,5.787764 0,0.8785 0.327284,1.825902 h -4.099666 q -0.0689,-0.413412 -0.0689,-0.947402 0,-0.465088 0.03445,-0.706245 -1.533069,1.102431 -3.031686,1.498617 -1.481392,0.378961 -3.479549,0.378961 -1.481392,0 -2.566598,-0.34451 -1.06798,-0.361735 -1.636421,-0.964627 -0.568442,-0.602892 -0.826824,-1.274686 -0.241157,-0.68902 -0.241157,-1.464167 0,-1.550294 0.654569,-2.773304 0.654568,-1.240235 1.843127,-2.067058 1.188559,-0.826824 2.704402,-1.343588 1.515843,-0.516765 3.35897,-0.740696 2.945559,-0.34451 5.115971,-1.067981 0.08613,-0.447862 0.08613,-0.999078 0,-1.240235 -0.757922,-1.808676 -0.740696,-0.568442 -2.377117,-0.568442 -1.636421,0 -2.876657,0.775147 -1.240235,0.775147 -1.808676,1.860353 l -3.565676,-0.516764 q 1.223009,-2.842206 3.514,-4.20302 2.308215,-1.360813 5.925568,-1.360813 3.083362,0 4.513078,0.912951 1.446941,0.895725 1.446941,3.255617 0,1.154108 -0.327284,2.669951 z m -12.609058,2.721627 q 0,0.465088 0.275608,0.809598 0.292833,0.34451 0.740696,0.53399 0.465088,0.172255 0.930177,0.258382 0.482313,0.0689 0.981852,0.0689 2.480471,0 3.961863,-1.050755 1.481392,-1.050754 1.929255,-3.324519 l 0.155029,-0.775147 q -1.016304,0.34451 -2.446019,0.585667 -1.41249,0.223931 -2.463245,0.361735 -1.050755,0.137804 -2.032608,0.396186 -0.981853,0.258383 -1.515843,0.775147 -0.516765,0.516765 -0.516765,1.360814 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58731-5" />
      <path
         d="m 73.081261,33.140434 q -1.291912,0 -2.342666,-0.223932 -1.050755,-0.206706 -1.757,-0.602892 -0.68902,-0.396186 -1.188559,-0.895725 -0.499539,-0.49954 -0.775147,-1.102432 -0.258382,-0.620117 -0.378961,-1.188558 -0.103353,-0.585667 -0.103353,-1.22301 0,-0.654569 0.0689,-1.119657 h 3.789608 q -0.01723,0.120579 -0.01723,0.34451 0,1.240235 0.895726,1.912029 0.895725,0.654569 2.532147,0.654569 1.94648,0 3.01446,-0.723471 1.067981,-0.740696 1.067981,-1.843127 0,-0.844049 -0.706245,-1.188559 -0.68902,-0.361735 -2.325442,-0.671794 -1.274686,-0.241157 -2.084284,-0.430637 -0.809598,-0.206706 -1.705323,-0.551216 -0.895726,-0.361735 -1.395265,-0.775147 -0.499539,-0.430637 -0.826823,-1.085206 -0.327284,-0.671794 -0.327284,-1.533068 0,-2.032608 1.136882,-3.565676 1.136882,-1.550294 2.928333,-2.308216 1.808676,-0.757921 3.944637,-0.757921 1.464166,0 2.566598,0.275607 1.119657,0.258383 1.791451,0.671795 0.671794,0.413411 1.085205,1.016303 0.430638,0.585667 0.585667,1.171334 0.155029,0.568441 0.155029,1.223009 0,0.568441 -0.120578,1.22301 h -3.789608 q 0.01723,-0.120578 0.01723,-0.327284 0,-1.808677 -2.807755,-1.808677 -1.825902,0 -2.721627,0.637344 -0.8785,0.637343 -0.8785,1.60197 0,0.826823 0.740696,1.205784 0.740696,0.378961 2.325441,0.68902 3.445098,0.654568 4.892039,1.567519 1.464167,0.912951 1.464167,2.652726 0,2.222088 -1.291912,3.875735 -1.274686,1.636421 -3.238392,2.428794 -1.94648,0.775147 -4.220245,0.775147 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58733-3" />
      <path
         d="m 91.667564,29.540306 -0.258383,3.32452 q -1.291911,0.258382 -2.601048,0.258382 -2.084285,0 -2.911108,-0.671794 -0.826824,-0.68902 -0.826824,-1.998157 0,-0.775147 0.447863,-2.911107 l 2.015382,-9.491245 h -2.204862 l 0.740696,-3.445098 h 2.204862 l 1.360814,-6.390656 h 3.789608 l -1.360814,6.390656 h 2.256539 l -0.740696,3.445098 h -2.256539 l -2.067059,9.732401 q -0.0689,0.292834 -0.0689,0.620118 0,1.274686 1.533069,1.274686 l 0.103353,-0.01722 q 0.103353,-0.01723 0.327284,-0.05168 0.241157,-0.03445 0.516765,-0.0689 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58735-5" />
      <path
         d="M 94.544221,32.899277 99.849672,7.9223177 h 18.259018 l -0.8096,3.7896073 h -14.1249 l -1.39526,6.614588 h 10.66257 l -0.80959,3.789608 h -10.66258 l -1.481394,6.993548 h 14.124904 l -0.8096,3.789608 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58737-6" />
      <path
         d="m 129.0641,22.908493 h -5.68441 l -2.11873,9.990784 h -4.13412 l 5.30545,-24.9769593 h 9.5257 q 2.30821,0 3.96186,0.3617352 1.65365,0.3617353 2.61827,1.0679803 0.96463,0.7062448 1.41249,1.6536468 0.44787,0.947402 0.44787,2.187637 0,4.426951 -2.98001,7.079676 -2.96279,2.6355 -8.35437,2.6355 z m 7.04523,-8.457715 q 0,-1.533069 -1.37804,-2.135961 -1.36081,-0.602892 -3.99631,-0.602892 h -4.97817 l -1.56752,7.406961 h 5.04707 q 6.87297,0 6.87297,-4.668108 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58739-2" />
      <path
         d="m 149.94139,32.899277 h -9.11228 l 5.30545,-24.9769593 h 8.69887 q 2.79053,0 4.77146,0.6028921 1.99816,0.5856666 3.13504,1.7225492 1.13688,1.119656 1.63642,2.549372 0.51677,1.429716 0.51677,3.290068 0,2.44602 -0.53399,4.805912 -0.51677,2.342666 -1.6881,4.547529 -1.17133,2.187637 -2.87666,3.841284 -1.70532,1.636421 -4.25469,2.6355 -2.53215,0.981853 -5.59829,0.981853 z m 0.43064,-3.789608 q 1.757,0 3.27284,-0.499539 1.51585,-0.516765 2.60105,-1.395265 1.08521,-0.895725 1.92926,-2.032607 0.84405,-1.136883 1.34358,-2.428794 0.51677,-1.309137 0.77515,-2.583824 0.25838,-1.291911 0.25838,-2.532147 0,-2.876656 -1.68809,-4.392499 -1.6881,-1.533069 -4.89204,-1.533069 h -4.51308 l -3.68626,17.397744 z"
         style="-inkscape-font-specification:'Loma Bold Oblique';fill:#ffffff"
         id="path58741-9" />
    </g>
  </g>
</svg> 

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

<code style="color:red">We are also launching a new [company called Marespa](https://www.marespa.es).es that will help EU citizens find an affordable apartment in Spain. With the price of Rent going through the roof in 2024, this might be the moment to make an investment, if you plan to work from the spanish coast. With this project we are also supporting our open source projects.</code>

We are thankful for the support and contributions so far!

## Interesting projects using this library

- [Bistable-smart-switch](https://github.com/martinberlin/bistable-smart-switch) an ESP32-C3 (or S3) smart switch for your wall
- [Invisible computers epaper](https://www.invisible-computers.com) your Google Calendar in an epaper display, drawn using CalEPD!

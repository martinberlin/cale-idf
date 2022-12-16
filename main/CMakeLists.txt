idf_component_register(
    # CALE official ESP-IDF Firmware
    
    #SRCS "cale.cpp"
    #SRCS "cale-7-color.cpp"

    # SRCS "demos/cale-sensor.cpp"
    # CALE with grayscale support (For now only plasticLogic)
    # Grayscale is also good to test parallel epaper with 16 gray levels
    #SRCS "cale-grayscale.cpp"

    # Generic demos for touch displays (With an added layer or Gdew027w3T that comes with touch mounted)
    #SRCS "demos/demo-epaper.cpp"
    #SRCS "demos/demo-grays.cpp"
    
    # TOUCH examples. 1- Touch indepently from Epd class uses Gdew027w3   2- Touch implemented in Gdew027w3T class
    #SRCS "demos/demo-touch.cpp"
    #SRCS "demos/demo-touch-epd-implemented.cpp"
    #SRCS "demos/demo-touch-keyboard.cpp"
    SRCS "demos/on-off-switch.cpp"

    # Generic demos for any displays
    #SRCS "demos/demo-sleep-clock.cpp"
    # SRCS "demo-sleep-clock-v2.cpp"
    #SRCS "demos/demo-fonts.cpp"

    # Demo to print 7 colors in Waveshare Acep epapers
    #SRCS "demos/demo-7-colors.cpp"

    # Demo only for plasticlogic.com epapers:
    #SRCS "demos/plasticlogic/demo-epaper-plasticlogic.cpp"
    #SRCS "demos/plasticlogic/demo-sleep-clock-temp.cpp"
    #SRCS "demos/plasticlogic/news-ticker.cpp"

    # Plasticlogic video demo: Please build it in www-image/video

    # Only parallel epapers (Driven by I2Sbus)
    #SRCS "demos/parallel/demo-epaper-parallel.cpp"
    #SRCS "demos/parallel/demo-epaper-parallel-partial.cpp"
    #SRCS "demos/parallel/demo-touch-parallel.cpp"
    #SRCS "demos/parallel/demo-touch-keyboard.cpp"
    
    # Multi-touch L58 in LilygoEPD47 (Requires epdiy)
    #SRCS "demos/epd47-touch/touch-simple.cpp"
    # Note: Enable in menuconfig > Touch FT6X36 Configuration > L58_MULTITOUCH to 1 
    #SRCS "demos/epd47-touch/multitouch-circles.cpp"
    
    INCLUDE_DIRS ".")

    # This is just to avoid that BitBank2 components include Arduino.h
idf_build_set_property(COMPILE_OPTIONS "-D __LINUX__" APPEND) 

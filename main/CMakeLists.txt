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
    
    # TOUCH examples. 1- Touch indepently from Epd class uses Gdew027w3   2- Touch implemented in Gdew027w3T class
    #SRCS "demos/demo-touch.cpp"
    #SRCS "demos/demo-touch-epd-implemented.cpp"
    
    #SRCS "demos/demo-touch-keyboard.cpp"

    # Generic demos for any displays
    # SRCS "demos/demo-sleep-clock.cpp"
    # SRCS "demo-sleep-clock-v2.cpp"
    # SRCS "demos/demo-fonts.cpp"

    # Demo to print 7 colors in Waveshare Acep epapers
    SRCS "demos/demo-7-colors.cpp"

    # Demo only for plasticlogic.com epapers:
    # SRCS "demos/demo-epaper-plasticlogic.cpp"

    # Only parallel epapers (Driven by I2Sbus)
    #SRCS "demos/demo-epaper-parallel.cpp"
    INCLUDE_DIRS ".")
    
set(COMPONENTS_DIRS, "components")
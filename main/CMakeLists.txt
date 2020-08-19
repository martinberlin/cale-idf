idf_component_register(
    # CALE official ESP-IDF Firmware
    #SRCS "cale.cpp"
    # CALE with grayscale support
    SRCS "cale-grayscale.cpp"

    # Generic demos for any displays
    #SRCS "demo-epaper.cpp"
    #SRCS "demo-sleep-clock.cpp"
    #SRCS "demo-sleep-clock-v2.cpp"
    #SRCS "demo-fonts.cpp"

    # Demo only for plasticlogic.com epapers:
    #SRCS "demo-epaper-plasticlogic.cpp"
    INCLUDE_DIRS ".")
    
set(COMPONENTS_DIRS, "components")
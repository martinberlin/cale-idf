idf_component_register(
    # CALE official ESP-IDF Firmware
    
    # SRCS "cale.cpp"

    SRCS "demos/cale-sensor.cpp"
    # CALE with grayscale support (For now only plasticLogic)
    # SRCS "cale-grayscale.cpp"

    # Generic demos for any displays

     SRCS "demos/demo-epaper.cpp"
    # SRCS "demos/demo-sleep-clock.cpp"
    # SRCS "demo-sleep-clock-v2.cpp"
    # SRCS "demos/demo-fonts.cpp"

    # Demo only for plasticlogic.com epapers:
    # SRCS "demos/demo-epaper-plasticlogic.cpp"
    INCLUDE_DIRS ".")
    
set(COMPONENTS_DIRS, "components")
idf_component_register(
    # Main CALE 
    #SRCS "cale.cpp"
    
    #SRCS "demo-sleep-clock.cpp"
    SRCS "demo-sleep-clock-v2.cpp"
    #SRCS "demo-fonts.cpp"
    SRCS "demo-epaper.cpp"

    #SRCS "demo-epaper-plasticlogic.cpp"
    INCLUDE_DIRS ".")
    
set(COMPONENTS_DIRS, "components")
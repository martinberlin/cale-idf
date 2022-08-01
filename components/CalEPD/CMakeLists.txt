# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "models/wave12i48.cpp"
    "models/gdew075HD.cpp"
    "models/gdew075T7.cpp"
    "models/gdew075T7Grays.cpp"
    "models/gdew075T8.cpp"
    "models/gdew0583t7.cpp"
    "models/gdew042t2.cpp"
    "models/gdew042t2Grays.cpp"
    "models/gdew027w3.cpp"
    "models/wave12i48.cpp"
    # Touch model, please also enable FT6X36-IDF in the REQUIRE section
    #"models/gdew027w3T.cpp"
    "models/gdew0213i5f.cpp"
    "models/gdep015OC1.cpp"
    "models/gdeh0154d67.cpp"
    "models/heltec0151.cpp"
    
    # 3 colors Goodisplay
    "models/color/gdeh0154z90.cpp"
    "models/color/gdew0583z21.cpp"
    "models/color/gdew075z09.cpp"
    "models/color/gdew075c64.cpp"
    "models/color/gdeh042Z96.cpp"
    "models/color/gdeh042Z21.cpp"
    "models/color/wave12i48BR.cpp"

    # 7 colors Waveshare
    "models/color/wave4i7Color.cpp"
    "models/color/wave5i7Color.cpp"

    # Plasticlogic.com Flex epaper models:
    "models/plasticlogic/epdspi2cs.cpp"
    "models/plasticlogic/plasticlogic.cpp"
    "models/plasticlogic/plasticlogic011.cpp"
    "models/plasticlogic/plasticlogic014.cpp"
    "models/plasticlogic/plasticlogic021.cpp"
    # Not-tested (never had one)
    #"models/plasticlogic/plasticlogic031.cpp"

    # Parallel epapers supported by Epdiy driver (Uncomment epdParallel & REQUIRES "epdiy")
    # Uncomment for parallel epapers:
    #"epdParallel.cpp"
    #"models/parallel/ED047TC1.cpp"
    #"models/parallel/ED047TC1touch.cpp"
    #"models/parallel/ED060SC4.cpp"
    #"models/parallel/ED060XCN.cpp" # pending to be added

    # Pending for more testing:
    "models/gdeh0213b73.cpp"
    # Common base classes
    "epd.cpp"
    "epd7color.cpp"
    "epdspi.cpp"
    "epd4spi.cpp"
    )

# If the project does not use a touch display component FT6X36-IDF can be removed or #commented
idf_component_register(SRCS ${srcs}      
                    REQUIRES "Adafruit-GFX"
                    REQUIRES "FT6X36-IDF"
                    # Uncomment for parallel epapers:
                    REQUIRES "epdiy"
                    INCLUDE_DIRS "include"
)
# Note about epdiy component: Even if it's not REQUIRED sometimes CMake includes it
#      giving errors. If that happens, just open the component CMakeLists and add:
#      return()
#      As the first line. This will tell CMake to not compile it.
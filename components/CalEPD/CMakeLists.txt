# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "models/wave12i48.cpp"
    "models/gdew075T7.cpp"
    "models/gdew075T8.cpp"
    "models/gdew0583t7.cpp"
    "models/gdew042t2.cpp"
    "models/gdew027w3.cpp"
    "models/gdew0213i5f.cpp"
    "models/gdep015OC1.cpp"
    "models/heltec0151.cpp"
    # Plasticlogic.com Flex epaper models:
    "models/plasticlogic/epdspi2cs.cpp"
    "models/plasticlogic/plasticlogic.cpp"
    "models/plasticlogic/plasticlogic011.cpp"
    "models/plasticlogic/plasticlogic014.cpp"
    "models/plasticlogic/plasticlogic021.cpp"
    "models/plasticlogic/plasticlogic031.cpp"

    # Pending for more testing:
    #   "models/gdeh0213b73.cpp"
    # Common base classes
    "epd.cpp"
    "epdspi.cpp"
    "epd4spi.cpp"
    )

idf_component_register(SRCS ${srcs}
                    REQUIRES "Adafruit-GFX"
                    INCLUDE_DIRS "include"
)


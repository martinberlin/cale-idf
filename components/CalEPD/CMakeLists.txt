# Add only the display srcs you are going to use:
set(srcs 
    "models/gdew075T7.cpp"
    "models/gdew042t2.cpp"
    "models/gdew0213i5f.cpp"
    "models/gdew027w3.cpp"
 # Pending to fix:
 #   "models/fix/gdeh0213b73.cpp"
    "epd.cpp"
    "epdspi.cpp"
    )

# Leave as an example of how to add target only for ESP32
# Only ESP32 has enough memory to do jpeg decoding
#if(IDF_TARGET STREQUAL "esp32")
#    list(APPEND srcs "decode_image.c")
#endif()

idf_component_register(SRCS ${srcs}
                    REQUIRES "Adafruit-GFX"
                    INCLUDE_DIRS "include"
)


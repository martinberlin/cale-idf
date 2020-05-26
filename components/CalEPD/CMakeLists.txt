set(srcs 
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


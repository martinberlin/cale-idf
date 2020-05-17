set(srcs "pretty_effect.c"
    "tjpgd.c"
    "epd.cpp"
    )

# Only ESP32 has enough memory to do jpeg decoding
if(IDF_TARGET STREQUAL "esp32")
    list(APPEND srcs "decode_image.c")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    EMBED_FILES image.jpg)


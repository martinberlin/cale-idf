set(srcs 
    # Common base classes that every model has to inherit
    "epd_driver_base.cpp"
    "i2s_data_bus.cpp"
    )

# Not sure what model is the Touch: To be confirmed later
idf_component_register(SRCS ${srcs}
                    REQUIRES "Adafruit-GFX"
                    REQUIRES "FT6X36-IDF"
                    INCLUDE_DIRS "include"
)
# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "FT6X36.cpp"
    "L58Touch.cpp"
    )

idf_component_register(SRCS ${srcs}
            REQUIRES driver esp_timer
            INCLUDE_DIRS "include"
)


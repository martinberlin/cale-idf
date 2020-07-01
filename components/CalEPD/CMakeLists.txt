# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "models/wave12i48.cpp"
    "models/gdew075T7.cpp"
    "models/gdew075T8.cpp"
    "models/gdew0583t7.cpp"
    "models/gdew042t2.cpp"
    "models/gdew027w3.cpp"
    "models/gdew0213i5f.cpp"
    
 # Pending for more testing:
 #   "models/gdeh0213b73.cpp"
    "epd.cpp"
    "epdspi.cpp"
    "epd4spi.cpp"
    )

idf_component_register(SRCS ${srcs}
                    REQUIRES "Adafruit-GFX"
                    INCLUDE_DIRS "include"
)


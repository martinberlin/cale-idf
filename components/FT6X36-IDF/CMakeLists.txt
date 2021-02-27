# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "FTttgo.cpp"
    "FT6X36.cpp"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
)


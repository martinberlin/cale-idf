# Make sure to check what sensor model you have
set(srcs 
    "bmp280/bmp280.c"
)
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "bmp280"
)
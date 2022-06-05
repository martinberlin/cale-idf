set(srcs 
    "bme280/bme280.c"
)
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "bme280"
)
set(srcs 
    # Common base classes
    "epd_driver.cpp"
    )

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS "include")
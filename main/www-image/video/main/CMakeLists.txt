# Uncomment only one app_source
set(app_sources "plasticlogic-video.cpp")

idf_component_register(SRCS ${app_sources} 
        REQUIRES CalEPD
)
spiffs_create_partition_image(spiffs ../spiffs FLASH_IN_PROJECT)
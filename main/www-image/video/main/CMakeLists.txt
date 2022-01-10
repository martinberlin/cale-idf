# Uncomment only one app_source
set(app_sources 
   #"plasticlogic-video.cpp"
   "epdiy-video.cpp"
   )

idf_component_register(SRCS ${app_sources} 
        REQUIRES CalEPD
        epdiy
        spiffs
)

# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'
spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
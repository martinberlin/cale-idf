# Uncomment only one app_source
#set(app_sources "jpg-render.cpp")

# Faster, uses less RAM, but looses dithering functionality:
#set(app_sources "jpg-render-v2.cpp")

# Only for IT8951 Eink dot com epapers with 16 bit parallel interface
#set(app_sources "jpg-render-it8951.cpp")

#set(app_sources "jpg-render-cinread8951.cpp")
set(app_sources "tests/cinread-8951-sensor-demo.cpp")
#set(app_sources "tests/bmp280-read.c")
#set(app_sources "tests/it8951-i2c-scan.cpp")
# Using @bitbank2/JPEGDEC
#set(app_sources "jpgdec-render.cpp")

#For SPI epaper tests
#REQUIRES CalEPD
idf_component_register(SRCS ${app_sources} 
        REQUIRES CalEPD jpegdec LovyanGFX
        nvs_flash esp-tls esp_http_client bosch
        # Embed the server root certificate into the final binary
        EMBED_TXTFILES ${project_dir}/ssl_cert/server_cert.pem
        )
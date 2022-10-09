# Uncomment only one app_source
#set(app_sources "jpg-render.cpp")

# Faster, uses less RAM, but looses dithering functionality:
#set(app_sources "jpg-render-v2.cpp")

# BLE render. Use CALE.es as a client to send the image
set(app_sources "ble-render.cpp")

# Using @bitbank2/JPEGDEC
#set(app_sources "jpgdec-render.cpp")

idf_component_register(SRCS ${app_sources} 
        REQUIRES CalEPD jpegdec
        nvs_flash esp-tls esp_http_client
        # IF using the BLE example ble-render
        bt
        # Embed the server root certificate into the final binary
        EMBED_TXTFILES ${project_dir}/ssl_cert/server_cert.pem
        )
set(app_sources "jpg-render.cpp")

idf_component_register(SRCS ${app_sources} 
        REQUIRES CalEPD 
        nvs_flash esp-tls esp_http_client
        # Embed the server root certificate into the final binary
        EMBED_TXTFILES ${project_dir}/ssl_cert/server_cert.pem
        )
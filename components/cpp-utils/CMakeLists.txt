set(COMPONENT_SRCDIRS "src")
set(COMPONENT_ADD_INCLUDEDIRS "src")
set(COMPONENT_REQUIRES
  "console"
  "fatfs"
  "json"
  "mdns"
  "nvs_flash"
)
set(COMPONENT_PRIV_REQUIRES)

register_component()

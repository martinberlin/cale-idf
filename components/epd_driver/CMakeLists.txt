idf_build_get_property(target IDF_TARGET)
# This component Requires a ESP32 target. Do not build if using this:
if(${target} STREQUAL "esp32c3")
        return()
endif()
if(${target} STREQUAL "esp32s2")
        return()
endif()

set(app_sources "epd_driver.c"
                "render.c"
                "display_ops.c"
                "font.c"
                "lut.c"
				"builtin_waveforms.c"
                "i2s_data_bus.c"
                "rmt_pulse.c"
				"highlevel.c"
				"epd_temperature.c")


idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "include" REQUIRES esp_adc_cal)

set_source_files_properties("lut.c" PROPERTIES COMPILE_OPTIONS -mno-fix-esp32-psram-cache-issue)

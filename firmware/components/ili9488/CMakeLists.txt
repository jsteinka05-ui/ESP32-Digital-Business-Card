# Build this driver as its own ESP-IDF component.
# Public headers are available to components that depend on ili9488.
idf_component_register(
    SRCS "ili9488.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_common
    PRIV_REQUIRES esp_driver_spi esp_driver_gpio freertos
)

idf_component_register(
    SRCS "main.c" "button.c"
    INCLUDE_DIRS "."
    REQUIRES freertos log esp_timer esp_driver_gpio
)

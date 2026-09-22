idf_component_register(
    SRCS "slides.c" "display_diagnostics.c" "main.c" "button.c"
    INCLUDE_DIRS "."
    REQUIRES freertos log esp_timer esp_driver_gpio esp_common esp_system heap ili9488
)

# Keep image symbols independent of their source directory
target_add_binary_data(${COMPONENT_LIB} "../images/business_card.rgb" BINARY
                       RENAME_TO "business_card_rgb")
target_add_binary_data(${COMPONENT_LIB} "../images/linkedin_qr.rgb" BINARY
                       RENAME_TO "linkedin_qr_rgb")

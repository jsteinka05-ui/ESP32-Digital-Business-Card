#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "button.h"
#include "ili9488.h"
#include "display_diagnostics.h"

void app_main(void)
{
    static const char *TAG = "business_card";
    ESP_ERROR_CHECK(button_init());
    ESP_LOGI(TAG, "Button input ready");

    // Set up the SPI connection before the panel
    ESP_ERROR_CHECK(ili9488_bus_init());
    // Drawing must be rejected until the panel is initialized
    if (ili9488_fill_rect(0, 0, 1, 1, 0, 0, 0) != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Uninitialized drawing check failed");
        ESP_ERROR_CHECK(ESP_FAIL);
    }
    ESP_ERROR_CHECK(ili9488_panel_init());
    ESP_LOGI(TAG, "LCD ready");
    // Run the display checks once and leave the pattern visible
    ESP_ERROR_CHECK(display_diagnostics_run());
    ESP_LOGI(TAG, "Button monitoring active");

    // Resume button sampling after display startup finishes
    while (true) {
        bool raw_pressed = button_is_pressed();
        button_event_t event = button_update(raw_pressed, esp_timer_get_time());

        switch (event) {
        case BUTTON_EVENT_PRESSED:
            ESP_LOGI(TAG, "Button pressed");
            break;
        case BUTTON_EVENT_RELEASED:
            ESP_LOGI(TAG, "Button released (no tracked press)");
            break;
        case BUTTON_EVENT_SHORT_PRESS:
            ESP_LOGI(TAG, "Short press released");
            break;
        case BUTTON_EVENT_LONG_PRESS:
            ESP_LOGI(TAG, "Long press released");
            break;
        case BUTTON_EVENT_NONE:
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

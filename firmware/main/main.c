#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "button.h"
#include "ili9488.h"
#include "display_diagnostics.h"
#include "slides.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"

static const char *TAG = "business_card";

static esp_err_t show_slide(size_t index)
{
    int64_t started_us = esp_timer_get_time();
    esp_err_t result = slides_draw(index);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Slide draw failed: %s", esp_err_to_name(result));
        return result;
    }
    // Record draw time and available memory after the transfer
    ESP_LOGI(TAG, "Slide %u: %s, %lld ms", (unsigned)index, slides_name(index),
             (long long)((esp_timer_get_time() - started_us) / 1000));
    ESP_LOGI(TAG, "Free heap: %lu, minimum: %lu, largest internal block: %u",
             (unsigned long)esp_get_free_heap_size(),
             (unsigned long)esp_get_minimum_free_heap_size(),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    return ESP_OK;
}

void app_main(void)
{
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
    // Keep the color checks available without running them every startup
#if CONFIG_BUSINESS_CARD_DISPLAY_DIAGNOSTICS
    ESP_ERROR_CHECK(display_diagnostics_run());
    vTaskDelay(pdMS_TO_TICKS(5000));
#endif
    ESP_ERROR_CHECK(slides_validate());
    size_t current_slide = 0;
    ESP_ERROR_CHECK(show_slide(current_slide));
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
        case BUTTON_EVENT_SHORT_PRESS: {
            ESP_LOGI(TAG, "Short press released");
            // Wrap after the final slide and update only after a complete draw
            size_t next_slide = (current_slide + 1) % slides_count();
            if (show_slide(next_slide) == ESP_OK) {
                current_slide = next_slide;
            }
            break;
        }
        case BUTTON_EVENT_LONG_PRESS:
            ESP_LOGI(TAG, "Long press released");
            break;
        case BUTTON_EVENT_NONE:
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

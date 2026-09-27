#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "button.h"
#include "ili9488.h"
#include "display_diagnostics.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"
#include "app_controller.h"
#include "transitions.h"
#include "power_manager.h"

static const char *TAG = "business_card";

static void log_memory(void)
{
    ESP_LOGI(TAG, "Free heap: %lu, minimum: %lu, largest internal block: %u",
             (unsigned long)esp_get_free_heap_size(),
             (unsigned long)esp_get_minimum_free_heap_size(),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

void app_main(void)
{
    ESP_ERROR_CHECK(button_init());
    ESP_ERROR_CHECK(power_manager_init());
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
    int64_t started_us = esp_timer_get_time();
    ESP_ERROR_CHECK(app_controller_init(started_us));
    app_controller_reset_activity(esp_timer_get_time());
    ESP_LOGI(TAG, "Initial slide ready in %lld ms",
             (long long)((esp_timer_get_time() - started_us) / 1000));
    log_memory();
    int64_t previous_sample_us = esp_timer_get_time();
    int64_t transition_started_us = 0;
    int64_t longest_gap_us = 0;
    int64_t transition_duration_us = 0;
    bool report_transition = false;
    ESP_LOGI(TAG, "Button monitoring active");

    // Resume button sampling after display startup finishes
    while (true) {
        bool raw_pressed = button_is_pressed();
        int64_t now_us = esp_timer_get_time();
        button_event_t event = button_update(raw_pressed, now_us);
        bool was_active = transitions_is_active();
        if ((was_active || report_transition) && now_us - previous_sample_us > longest_gap_us) {
            longest_gap_us = now_us - previous_sample_us;
        }
        previous_sample_us = now_us;
        if (report_transition) {
            // Wait for this sample to include the final step and task delay
            ESP_LOGI(TAG, "Transition ended in %lld ms, longest sampled gap %lld us",
                     (long long)(transition_duration_us / 1000),
                     (long long)longest_gap_us);
            log_memory();
            report_transition = false;
        }

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

        esp_err_t result = app_controller_update(event, raw_pressed, now_us);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Display operation failed: %s", esp_err_to_name(result));
        }
        if (!was_active && transitions_is_active()) {
            transition_started_us = now_us;
            longest_gap_us = 0;
        } else if (was_active && !transitions_is_active()) {
            transition_duration_us = esp_timer_get_time() - transition_started_us;
            report_transition = true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

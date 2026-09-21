#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "button.h"

void app_main(void)
{
    static const char *TAG = "business_card";
    ESP_ERROR_CHECK(button_init());
    ESP_LOGI(TAG, "Button input ready");

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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "button.h"

void app_main(void)
{
    static const char *TAG = "business_card";
    ESP_ERROR_CHECK(button_init());
    bool pressed;
    bool prev_pressed = button_is_pressed();

    while(true) {
        pressed = button_is_pressed();
        // Report raw state changes; contact bounce is not filtered yet.
        if (pressed != prev_pressed) {
            ESP_LOGI(TAG, "Status: %d", pressed);
            prev_pressed = pressed;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

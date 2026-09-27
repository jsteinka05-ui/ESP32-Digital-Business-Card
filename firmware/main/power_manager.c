#include "power_manager.h"
#include "button.h"
#include "ili9488.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stddef.h>

#ifdef CONFIG_PM_POWER_DOWN_PERIPHERAL_IN_LIGHT_SLEEP
#error "GPIO6 wake requires the GPIO peripheral to remain powered"
#endif

static const char *TAG = "card_power";
static const gpio_num_t WAKE_BUTTON = GPIO_NUM_6;
static const int64_t IDLE_US = 300000000;
static const int64_t RELEASE_US = 30000;
static const int64_t PANEL_SETTLE_US = 120000;
static bool initialized;
static bool preparing;
static bool cancel_sleep;
static bool release_pending;
static int64_t released_since_us;
static int64_t prepared_activity_us;
static int64_t panel_ready_at_us;
static esp_err_t prepare_result;

esp_err_t power_manager_init(void)
{
    if (initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    // Use the normal input configuration during ordinary light sleep
    esp_err_t result = gpio_sleep_sel_dis(WAKE_BUTTON);
    if (result != ESP_OK) {
        return result;
    }
    result = gpio_wakeup_enable(WAKE_BUTTON, GPIO_INTR_LOW_LEVEL);
    if (result != ESP_OK) {
        return result;
    }
    result = esp_sleep_enable_gpio_wakeup();
    if (result != ESP_OK) {
        gpio_wakeup_disable(WAKE_BUTTON);
        return result;
    }
    initialized = true;
    return ESP_OK;
}

bool power_manager_is_preparing(void)
{
    return preparing;
}

esp_err_t power_manager_update(int64_t now_us, int64_t last_activity_us,
                               bool can_sleep, bool raw_pressed, power_event_t *event)
{
    if (event == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *event = POWER_EVENT_NONE;
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (preparing) {
        // Remember even a brief press while the panel settles
        if (!can_sleep || raw_pressed || last_activity_us != prepared_activity_us) {
            cancel_sleep = true;
        }
        if (now_us < panel_ready_at_us) {
            return ESP_OK;
        }
        preparing = false;
        release_pending = false;
        *event = POWER_EVENT_RESTORE;
        if (cancel_sleep || button_is_pressed()) {
            ESP_LOGI(TAG, "Sleep entry cancelled");
            return prepare_result;
        }

        // A press racing this call either rejects sleep or wakes it immediately
        ESP_LOGI(TAG, "Entering light sleep");
        esp_err_t result = esp_light_sleep_start();
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Wake cause: %d", (int)esp_sleep_get_wakeup_cause());
        }
        return result;
    }

    if (!can_sleep || raw_pressed) {
        release_pending = false;
        return ESP_OK;
    }
    if (!release_pending) {
        release_pending = true;
        released_since_us = now_us;
    }
    if (now_us < last_activity_us || now_us - last_activity_us < IDLE_US ||
        now_us - released_since_us < RELEASE_US) {
        return ESP_OK;
    }
    // Recheck the pin after the application's earlier sample
    if (button_is_pressed()) {
        release_pending = false;
        return ESP_OK;
    }

    prepare_result = ili9488_panel_sleep();
    panel_ready_at_us = esp_timer_get_time() + PANEL_SETTLE_US;
    prepared_activity_us = last_activity_us;
    cancel_sleep = prepare_result != ESP_OK;
    preparing = true;
    ESP_LOGI(TAG, "Inactivity timeout reached");
    return ESP_OK;
}

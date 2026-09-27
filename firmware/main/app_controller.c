#include "app_controller.h"
#include "slides.h"
#include "transitions.h"
#include "ili9488.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "power_manager.h"

static const char *TAG = "card_control";
static const int64_t RELEASE_US = 30000;
static size_t current_slide;
static size_t target_slide;
static int64_t last_activity_us;
static bool initialized;
static bool button_held;
static bool input_blocked;
static bool release_pending;
static int64_t released_since_us;

static void block_input(void)
{
    input_blocked = true;
    release_pending = false;
}

size_t app_controller_current_slide(void)
{
    return current_slide;
}

int64_t app_controller_last_activity_us(void)
{
    return last_activity_us;
}

void app_controller_reset_activity(int64_t now_us)
{
    last_activity_us = now_us;
}

static esp_err_t restore_slide(void)
{
    block_input();
    esp_err_t result = ili9488_panel_init();
    if (result == ESP_OK) {
        result = slides_draw(current_slide);
    }
    // Restart the timeout after drawing or a failed attempt
    last_activity_us = esp_timer_get_time();
    return result;
}

static esp_err_t update_power(bool raw_pressed, int64_t now_us)
{
    power_event_t event;
    bool can_sleep = !input_blocked && !button_held && !transitions_is_active();
    esp_err_t result = power_manager_update(now_us, last_activity_us,
                                            can_sleep, raw_pressed, &event);
    if (event == POWER_EVENT_RESTORE) {
        int64_t restored_at_us = esp_timer_get_time();
        esp_err_t restore_result = restore_slide();
        if (restore_result != ESP_OK) {
            return restore_result;
        }
        ESP_LOGI(TAG, "Slide restored: %s, redraw %lld ms", slides_name(current_slide),
                 (long long)((esp_timer_get_time() - restored_at_us) / 1000));
    }
    return result;
}

esp_err_t app_controller_init(int64_t now_us)
{
    initialized = false;
    button_held = false;
    transitions_cancel();
    current_slide = 0;
    target_slide = 0;
    last_activity_us = now_us;
    block_input();
    esp_err_t result = slides_validate();
    if (result == ESP_OK) {
        result = slides_draw(current_slide);
    }
    if (result == ESP_OK) {
        initialized = true;
    }
    return result;
}

esp_err_t app_controller_update(button_event_t event, bool raw_pressed, int64_t now_us)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    // Accepted activity still counts when its action is suppressed
    if (event != BUTTON_EVENT_NONE) {
        last_activity_us = now_us;
    }
    if (event == BUTTON_EVENT_PRESSED) {
        button_held = true;
    } else if (event == BUTTON_EVENT_RELEASED || event == BUTTON_EVENT_SHORT_PRESS ||
               event == BUTTON_EVENT_LONG_PRESS) {
        button_held = false;
    }
    // Keep polling input during panel settling without dispatching gestures
    if (power_manager_is_preparing()) {
        return update_power(raw_pressed, now_us);
    }

    if (transitions_is_active()) {
        esp_err_t result = transitions_update(now_us);
        if (!transitions_is_active()) {
            block_input();
            if (result == ESP_OK) {
                current_slide = target_slide;
                ESP_LOGI(TAG, "Transition complete: %s", slides_name(current_slide));
            }
        }
        return result;
    }

    if (input_blocked) {
        // Require a stable release after drawing before accepting new gestures
        if (raw_pressed || event == BUTTON_EVENT_PRESSED) {
            release_pending = false;
        } else if (!release_pending) {
            release_pending = true;
            released_since_us = now_us;
        } else if (now_us - released_since_us >= RELEASE_US) {
            input_blocked = false;
            release_pending = false;
        }
        return ESP_OK;
    }

    if (event == BUTTON_EVENT_SHORT_PRESS) {
        target_slide = (current_slide + 1) % slides_count();
        esp_err_t result = transitions_start(target_slide, now_us);
        if (result == ESP_OK) {
            block_input();
            ESP_LOGI(TAG, "Transition started: %s", slides_name(target_slide));
        }
        return result;
    }
    if (event == BUTTON_EVENT_LONG_PRESS) {
        // Reuse SPI and redraw without advancing the selected slide
        esp_err_t result = restore_slide();
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "LCD recovery complete: %s", slides_name(current_slide));
        }
        return result;
    }
    return update_power(raw_pressed, now_us);
}

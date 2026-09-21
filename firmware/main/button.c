#include "button.h"
#include "driver/gpio.h"

static const gpio_num_t BUTTON_6 = GPIO_NUM_6;

static const int64_t DEBOUNCE_US = 30000;
static const int64_t LONG_PRESS_US = 900000;

// If first button reading has init. stored states
static bool initialized;
// Latest raw button state checked for stability
static bool current_pressed;
// Button state that is stable after debounce time
static bool stable_pressed;
// If a valid press is being timed for long vs short classification
static bool press_active;
// timestamp of most recent raw time change in us
static int64_t time_changed_us;
// timestamp when a debounced press is validated in us
static int64_t pressed_at_us;

esp_err_t button_init(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_6),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t result = gpio_config(&button_config);
    if (result == ESP_OK) {
        initialized = false;
        press_active = false;
    }
    return result;
}

bool button_is_pressed(void)
{
    // Active-low: pressing the button connects GPIO6 to ground.
    return (gpio_get_level(BUTTON_6) == 0);
}

button_event_t button_update(bool raw_pressed, int64_t curr_us)
{
    if (!initialized) {
        current_pressed = raw_pressed;
        stable_pressed = raw_pressed;
        time_changed_us = curr_us;
        press_active = false;

        
        initialized = true;
        return BUTTON_EVENT_NONE;
    }

    if (raw_pressed != current_pressed) {
        current_pressed = raw_pressed;
        time_changed_us = curr_us;
    }

    if (current_pressed == stable_pressed || curr_us - time_changed_us < DEBOUNCE_US) {
        return BUTTON_EVENT_NONE;
    }

    // Button state now stable, done debouncing
    stable_pressed = current_pressed;
    if (stable_pressed) {
        pressed_at_us = curr_us;
        press_active = true;
        return BUTTON_EVENT_PRESSED;
    }

    // If button held at startup initlization stores high, releases it here
    if (!press_active) {
        return BUTTON_EVENT_RELEASED;
    }

    press_active = false;
    // For long vs short press later implementation
    return curr_us - pressed_at_us >= LONG_PRESS_US ? BUTTON_EVENT_LONG_PRESS : BUTTON_EVENT_SHORT_PRESS;
}


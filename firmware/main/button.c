#include "button.h"
#include "driver/gpio.h"

static const gpio_num_t BUTTON_6 = GPIO_NUM_6;

esp_err_t button_init(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_6),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    return gpio_config(&button_config);
}

bool button_is_pressed(void)
{
    // Active-low: pressing the button connects GPIO6 to ground.
    return (gpio_get_level(BUTTON_6) == 0);
}

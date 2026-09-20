#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "esp_err.h"

// Configure the button input once at startup.
// Return ESP_OK on success, or the error returned by gpio_config().
esp_err_t button_init(void);

// Read the raw button state: true means pressed, false means released.
// Call only after successful initialization. Debouncing comes later.
bool button_is_pressed(void);

#endif // BUTTON_H

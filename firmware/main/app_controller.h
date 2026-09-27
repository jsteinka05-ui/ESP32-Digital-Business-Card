#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "button.h"
#include "esp_err.h"

// Display the first slide after the panel is initialized
esp_err_t app_controller_init(int64_t now_us);

// Process input and advance one animation step
esp_err_t app_controller_update(button_event_t event, bool raw_pressed, int64_t now_us);

size_t app_controller_current_slide(void);
int64_t app_controller_last_activity_us(void);

// Start a fresh inactivity interval after startup drawing finishes
void app_controller_reset_activity(int64_t now_us);

#endif // APP_CONTROLLER_H

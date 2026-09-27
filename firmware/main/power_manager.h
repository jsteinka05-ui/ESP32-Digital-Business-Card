#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef enum {
    POWER_EVENT_NONE,
    POWER_EVENT_RESTORE
} power_event_t;

// Configure active-low GPIO6 wake with its normal pull-up retained
esp_err_t power_manager_init(void);

bool power_manager_is_preparing(void);

// Check inactivity and advance sleep entry without queuing button actions
// RESTORE requests a redraw after wake, cancellation, or a failed sleep attempt
esp_err_t power_manager_update(int64_t now_us, int64_t last_activity_us,
                               bool can_sleep, bool raw_pressed, power_event_t *event);

#endif // POWER_MANAGER_H

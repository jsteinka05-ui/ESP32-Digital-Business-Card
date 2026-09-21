#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Events describe accepted states
typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

// Configure the button input once at startup.

esp_err_t button_init(void);

// Read the raw button state
bool button_is_pressed(void);

// Process one raw reading and a timestamp in microseconds
// Return a transition once the reading has remained stable for 30 ms.
// The first reading establishes the initial state without generating an event.
// A tracked release returns SHORT_PRESS (<900 ms) or LONG_PRESS (>=900 ms),
// replacing RELEASED for that transition. RELEASED alone means no tracked press
button_event_t button_update(bool raw_pressed, int64_t now_us);

#endif // BUTTON_H

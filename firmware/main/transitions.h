#ifndef TRANSITIONS_H
#define TRANSITIONS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

// Start a black wipe followed by a shuffled tile reveal
esp_err_t transitions_start(size_t slide, int64_t now_us);

// Draw one bounded step when its timestamp is due
esp_err_t transitions_update(int64_t now_us);

bool transitions_is_active(void);

// Stop work without changing the selected slide
void transitions_cancel(void);

#endif // TRANSITIONS_H

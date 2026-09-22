#ifndef SLIDES_H
#define SLIDES_H

#include <stddef.h>
#include "esp_err.h"

// Number of embedded slides in display order
size_t slides_count(void);

// Return the slide name or NULL for an invalid index
const char *slides_name(size_t index);

// Check every embedded image before displaying the first slide
esp_err_t slides_validate(void);

// Draw a slide without resetting the panel
esp_err_t slides_draw(size_t index);

#endif // SLIDES_H

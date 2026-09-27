#ifndef ILI9488_H
#define ILI9488_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#define ILI9488_WIDTH 320
#define ILI9488_HEIGHT 480

// Configure control pins and SPI once. Repeated calls return INVALID_STATE.
esp_err_t ili9488_bus_init(void);

// Reset and configure the panel using the existing SPI connection.
// Does not clear the screen. Drawing requires successful panel initialization.
esp_err_t ili9488_panel_init(void);

// Turn off the display and request panel sleep
// Allow at least 120 ms before sleeping the MCU or reinitializing the panel
esp_err_t ili9488_panel_sleep(void);

// Fill a contained rectangle in native 320 x 480 coordinates.
// Reject negative coordinates, nonpositive sizes, and out-of-bounds rectangles.
// Colors are 0..255 and converted to RGB666 during transmission.
esp_err_t ili9488_fill_rect(int x, int y, int width, int height,
                          uint8_t red, uint8_t green, uint8_t blue);

// Fill the entire panel with one color.
esp_err_t ili9488_fill_screen(uint8_t red, uint8_t green, uint8_t blue);

// Draw packed RGB888 rows from flash or RAM through the shared row buffer
// Length must match width times height times three with no padding
esp_err_t ili9488_draw_rgb888(int x, int y, int width, int height,
                            const uint8_t *pixels, size_t length);

// Display calls are synchronous and must all come from the same task.

#endif // ILI9488_H

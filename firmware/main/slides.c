#include "slides.h"
#include "ili9488.h"
#include <stdint.h>
#include <stdbool.h>

// Linker symbols point directly to the images stored in flash
extern const uint8_t business_card_start[] asm("_binary_business_card_rgb_start");
extern const uint8_t business_card_end[] asm("_binary_business_card_rgb_end");
extern const uint8_t linkedin_qr_start[] asm("_binary_linkedin_qr_rgb_start");
extern const uint8_t linkedin_qr_end[] asm("_binary_linkedin_qr_rgb_end");

typedef struct {
    const char *name;
    const uint8_t *start;
    const uint8_t *end;
} slide_t;

// Business card first and QR code second
static const slide_t slides[] = {
    {"Business card", business_card_start, business_card_end},
    {"LinkedIn QR", linkedin_qr_start, linkedin_qr_end}
};

static size_t image_length(const slide_t *slide)
{
    // Linker addresses are not ordinary C array boundaries
    return (uintptr_t)slide->end - (uintptr_t)slide->start;
}

size_t slides_count(void)
{
    return sizeof(slides) / sizeof(slides[0]);
}

const char *slides_name(size_t index)
{
    if (index >= slides_count()) {
        return NULL;
    }
    return slides[index].name;
}

esp_err_t slides_validate(void)
{
    size_t expected_length = (size_t)ILI9488_WIDTH * ILI9488_HEIGHT * 3;
    for (size_t index = 0; index < slides_count(); index++) {
        if (image_length(&slides[index]) != expected_length) {
            return ESP_ERR_INVALID_SIZE;
        }
    }
    return ESP_OK;
}

esp_err_t slides_read_region(size_t index, int x, int y, int width, int height,
                            uint8_t *pixels, size_t length)
{
    // Validate before multiplying sizes or reading image bytes
    if (index >= slides_count() || pixels == NULL || x < 0 || y < 0 ||
        x >= ILI9488_WIDTH || y >= ILI9488_HEIGHT || width <= 0 || height <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (width > ILI9488_WIDTH - x || height > ILI9488_HEIGHT - y) {
        return ESP_ERR_INVALID_ARG;
    }
    if (length != (size_t)width * height * 3) {
        return ESP_ERR_INVALID_SIZE;
    }
    esp_err_t result = slides_validate();
    if (result != ESP_OK) {
        return result;
    }

    const slide_t *slide = &slides[index];
    const uint8_t *background = business_card_start;
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width; column++) {
            int source_x = x + column;
            int source_y = y + row;
            size_t source_offset = ((size_t)source_y * ILI9488_WIDTH + source_x) * 3;
            size_t destination = ((size_t)row * width + column) * 3;
            bool inside_panel = source_x >= 25 && source_x < 295 &&
                                source_y >= 105 && source_y < 375;
            for (int channel = 0; channel < 3; channel++) {
                uint8_t color = slide->start[source_offset + channel];
                if (index == 1) {
                    color = background[channel];
                    if (inside_panel) {
                        // Use the same gold and translucent white panel
                        uint8_t panel = (3 * background[channel] + 255) / 4;
                        color = (uint16_t)slide->start[source_offset + channel] * panel / 255;
                    }
                }
                pixels[destination + channel] = color;
            }
        }
    }
    return ESP_OK;
}

static esp_err_t draw_qr_slide(size_t index)
{
    uint8_t row_pixels[ILI9488_WIDTH * 3];
    for (int y = 0; y < ILI9488_HEIGHT; y++) {
        esp_err_t result = slides_read_region(index, 0, y, ILI9488_WIDTH, 1,
                                              row_pixels, sizeof(row_pixels));
        if (result != ESP_OK) {
            return result;
        }
        result = ili9488_draw_rgb888(0, y, ILI9488_WIDTH, 1,
                                     row_pixels, sizeof(row_pixels));
        if (result != ESP_OK) {
            return result;
        }
    }
    return ESP_OK;
}

esp_err_t slides_draw(size_t index)
{
    if (index >= slides_count()) {
        return ESP_ERR_INVALID_ARG;
    }
    const slide_t *slide = &slides[index];
    if (index == 1) {
        return draw_qr_slide(index);
    }
    return ili9488_draw_rgb888(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT,
                              slide->start, image_length(slide));
}

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

static esp_err_t draw_qr_slide(const slide_t *slide)
{
    size_t expected_length = (size_t)ILI9488_WIDTH * ILI9488_HEIGHT * 3;
    if (image_length(slide) != expected_length) {
        return ESP_ERR_INVALID_SIZE;
    }

    // Match the plain gold at the corner of the business card
    const uint8_t *background = business_card_start;
    uint8_t row_pixels[ILI9488_WIDTH * 3];
    for (int y = 0; y < ILI9488_HEIGHT; y++) {
        for (int x = 0; x < ILI9488_WIDTH; x++) {
            size_t source_offset = ((size_t)y * ILI9488_WIDTH + x) * 3;
            // QR panel bounds in the rotated native image
            bool inside_panel = x >= 25 && x < 295 && y >= 105 && y < 375;
            for (int channel = 0; channel < 3; channel++) {
                uint8_t color = background[channel];
                if (inside_panel) {
                    // Blend white at 25 percent then retain the original QR pattern
                    uint8_t panel = (3 * background[channel] + 255) / 4;
                    color = (uint16_t)slide->start[source_offset + channel] * panel / 255;
                }
                row_pixels[x * 3 + channel] = color;
            }
        }
        esp_err_t result = ili9488_draw_rgb888(0, y, ILI9488_WIDTH, 1,
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
        return draw_qr_slide(slide);
    }
    return ili9488_draw_rgb888(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT,
                              slide->start, image_length(slide));
}

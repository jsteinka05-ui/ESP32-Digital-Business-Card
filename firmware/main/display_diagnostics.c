#include "display_diagnostics.h"
#include "ili9488.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <limits.h>
#include <stddef.h>

static const char *TAG = "display_test";

static esp_err_t check_invalid_rectangles(void)
{
    // Each case holds x y width and height
    // Include invalid edges and values that could overflow
    static const int rectangles[][4] = {
        {-1, 0, 1, 1}, {0, -1, 1, 1}, {0, 0, 0, 1}, {0, 0, 1, 0},
        {0, 0, -1, 1}, {0, 0, 1, -1}, {320, 0, 1, 1}, {0, 480, 1, 1},
        {319, 479, 2, 1}, {319, 479, 1, 2}, {0, 0, INT_MAX, 1},
        {INT_MAX, 0, 1, 1}, {0, INT_MAX, 1, 1}, {0, 0, 1, INT_MAX},
        {INT_MIN, 0, 1, 1}, {0, INT_MIN, 1, 1}
    };

    for (size_t i = 0; i < sizeof(rectangles) / sizeof(rectangles[0]); i++) {
        esp_err_t result = ili9488_fill_rect(rectangles[i][0], rectangles[i][1],
                                           rectangles[i][2], rectangles[i][3], 255, 0, 0);
        // Invalid arguments are expected here, not a test failure
        if (result != ESP_ERR_INVALID_ARG) {
            ESP_LOGE(TAG, "Bounds case %u failed: %s", (unsigned)i, esp_err_to_name(result));
            return ESP_FAIL;
        }
    }
    ESP_LOGI(TAG, "Invalid rectangles rejected");
    return ESP_OK;
}

static esp_err_t show_color_fills(void)
{
    // Color names match the expected screen output
    static const struct {
        const char *name;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } colors[] = {
        {"Black", 0, 0, 0}, {"Red", 255, 0, 0}, {"Green", 0, 255, 0},
        {"Blue", 0, 0, 255}, {"White", 255, 255, 255}
    };

    for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
        ESP_LOGI(TAG, "Fill: %s", colors[i].name);
        esp_err_t result = ili9488_fill_screen(colors[i].red, colors[i].green, colors[i].blue);
        if (result != ESP_OK) {
            return result;
        }
        // Hold each fill long enough to inspect
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    return ESP_OK;
}

static esp_err_t show_corner_pattern(void)
{
    // Clear the previous fill before drawing markers
    esp_err_t result = ili9488_fill_screen(0, 0, 0);
    if (result != ESP_OK) {
        return result;
    }

    // Four corners followed by the center box and thin lines
    static const struct {
        int x, y, width, height;
        uint8_t red, green, blue;
    } rectangles[] = {
        {0, 0, 20, 20, 255, 0, 0},
        {ILI9488_WIDTH - 20, 0, 20, 20, 0, 255, 0},
        {0, ILI9488_HEIGHT - 20, 20, 20, 0, 0, 255},
        {ILI9488_WIDTH - 20, ILI9488_HEIGHT - 20, 20, 20, 255, 255, 255},
        {(ILI9488_WIDTH - 80) / 2, (ILI9488_HEIGHT - 40) / 2, 80, 40, 255, 255, 0},
        {0, 120, ILI9488_WIDTH, 1, 0, 255, 255},
        {80, 0, 1, ILI9488_HEIGHT, 255, 0, 255}
    };

    for (size_t i = 0; i < sizeof(rectangles) / sizeof(rectangles[0]); i++) {
        result = ili9488_fill_rect(rectangles[i].x, rectangles[i].y,
                                  rectangles[i].width, rectangles[i].height,
                                  rectangles[i].red, rectangles[i].green, rectangles[i].blue);
        if (result != ESP_OK) {
            return result;
        }
    }
    // Log native coordinates for comparison with the mounted display
    ESP_LOGI(TAG, "Native corners: (0,0) red; (319,0) green; (0,479) blue; (319,479) white marker");
    ESP_LOGI(TAG, "Center: yellow 80x40; cyan horizontal line; magenta vertical line");
    return ESP_OK;
}

esp_err_t display_diagnostics_run(void)
{
    // A second bus setup must not allocate another device
    if (ili9488_bus_init() != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Repeated bus initialization was not rejected");
        return ESP_FAIL;
    }
    esp_err_t result = check_invalid_rectangles();
    if (result != ESP_OK) {
        return result;
    }
    result = show_color_fills();
    if (result != ESP_OK) {
        return result;
    }
    // Verify panel initialization can reuse the existing SPI connection
    result = ili9488_panel_init();
    if (result != ESP_OK) {
        return result;
    }
    result = show_corner_pattern();
    if (result != ESP_OK) {
        return result;
    }
    ESP_LOGI(TAG, "Software checks passed; inspect LCD colors and orientation");
    return ESP_OK;
}

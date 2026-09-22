#include "ili9488.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <stdbool.h>
#include <stddef.h>
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// LCD connections on the assembled PCB
static const gpio_num_t LCD_SCK = GPIO_NUM_8;
static const gpio_num_t LCD_MOSI = GPIO_NUM_10;
static const gpio_num_t LCD_CS = GPIO_NUM_4;
static const gpio_num_t LCD_DC = GPIO_NUM_5;
static const gpio_num_t LCD_RESET = GPIO_NUM_3;
static const spi_host_device_t LCD_HOST = SPI2_HOST;

// Transfer size and LCD command values
enum {
    TRANSFER_BYTES = ILI9488_WIDTH * 3,
    SWRESET = 0x01,
    SLPOUT = 0x11,
    DISPON = 0x29,
    CASET = 0x2A,
    PASET = 0x2B,
    RAMWR = 0x2C,
    MADCTL = 0x36,
    COLMOD = 0x3A
};

// Handle used for all LCD transfers
static spi_device_handle_t lcd_device;
// If this driver owns the SPI bus
static bool bus_initialized;
// If the panel completed its reset and configuration
static bool panel_ready;
// One row of RGB666 pixels, reused for each transfer
static DMA_ATTR uint8_t row_buffer[TRANSFER_BYTES];

static esp_err_t configure_control_pins(void)
{
    // DC selects command or data and RESET restarts the panel
    gpio_config_t control_config = {
        .pin_bit_mask = (1ULL << LCD_DC) | (1ULL << LCD_RESET),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t result = gpio_config(&control_config);
    if (result != ESP_OK) {
        return result;
    }
    result = gpio_set_level(LCD_DC, 0);
    if (result != ESP_OK) {
        return result;
    }
    return gpio_set_level(LCD_RESET, 1);
}

static esp_err_t initialize_spi_bus(void)
{
    // Unused SPI pins are set to -1
    spi_bus_config_t bus_config = {
        .mosi_io_num = LCD_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = LCD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = TRANSFER_BYTES
    };
    return spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO);
}

static esp_err_t register_lcd_device(void)
{
    // Send at 20 MHz and let the SPI driver control CS
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 20000000,
        .mode = 0,
        .spics_io_num = LCD_CS,
        .queue_size = 1
    };
    return spi_bus_add_device(LCD_HOST, &device_config, &lcd_device);
}

esp_err_t ili9488_bus_init(void)
{
    // Prevent registering the same bus twice
    if (bus_initialized || lcd_device != NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t result = configure_control_pins();
    if (result != ESP_OK) {
        return result;
    }
    result = initialize_spi_bus();
    if (result != ESP_OK) {
        return result;
    }
    bus_initialized = true;

    result = register_lcd_device();
    if (result != ESP_OK) {
        lcd_device = NULL;
        // Free only the bus allocated by this call
        if (spi_bus_free(LCD_HOST) == ESP_OK) {
            bus_initialized = false;
        }
        return result;
    }
    return ESP_OK;
}

static esp_err_t write_bytes(bool is_data, const uint8_t *bytes, size_t length)
{
    if (lcd_device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Commands with no parameters need no data transfer
    if (length == 0) {
        return ESP_OK;
    }
    if (bytes == NULL || length > TRANSFER_BYTES) {
        return ESP_ERR_INVALID_ARG;
    }
    // DC low for commands and high for data
    esp_err_t result = gpio_set_level(LCD_DC, is_data ? 1 : 0);
    if (result != ESP_OK) {
        return result;
    }

    // SPI length is in bits, buffer length is in bytes
    spi_transaction_t transaction = {
        .length = length * 8,
        .tx_buffer = bytes
    };
    // Wait until the transfer finishes before reusing the buffer
    return spi_device_polling_transmit(lcd_device, &transaction);
}

static esp_err_t write_command(uint8_t command)
{
    return write_bytes(false, &command, 1);
}

static esp_err_t write_data(const uint8_t *data, size_t length)
{
    return write_bytes(true, data, length);
}

static esp_err_t write_command_data(uint8_t command, const uint8_t *data, size_t length)
{
    if (length > TRANSFER_BYTES || (length != 0 && data == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }
    // Send the command before its parameter bytes
    esp_err_t result = write_command(command);
    if (result != ESP_OK) {
        return result;
    }
    return write_data(data, length);
}

static esp_err_t hardware_reset(void)
{
    // Reset delays retained from the working prototype
    esp_err_t result = gpio_set_level(LCD_RESET, 1);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    // Pull RESET low then release it high
    result = gpio_set_level(LCD_RESET, 0);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    result = gpio_set_level(LCD_RESET, 1);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(300));
    return ESP_OK;
}

esp_err_t ili9488_panel_init(void)
{
    // Block drawing until every setup command succeeds
    panel_ready = false;
    if (lcd_device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t result = hardware_reset();
    if (result != ESP_OK) {
        return result;
    }
    result = write_command(SWRESET);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(150));
    // Wake the panel before setting its display format
    result = write_command(SLPOUT);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(150));

    // Native 320 x 480 coordinates, page reversal and BGR color order
    uint8_t orientation = 0x88;
    result = write_command_data(MADCTL, &orientation, 1);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    // RGB666 uses three bytes for each pixel
    uint8_t pixel_format = 0x66;
    result = write_command_data(COLMOD, &pixel_format, 1);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    // Enable the display after its settings are applied
    result = write_command(DISPON);
    if (result != ESP_OK) {
        return result;
    }
    vTaskDelay(pdMS_TO_TICKS(120));
    panel_ready = true;
    return ESP_OK;
}

static esp_err_t set_address_window(int x, int y, int width, int height)
{
    // Check origin first so the size checks cannot overflow
    if (x < 0 || y < 0 || x >= ILI9488_WIDTH || y >= ILI9488_HEIGHT ||
        width <= 0 || height <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (width > ILI9488_WIDTH - x || height > ILI9488_HEIGHT - y) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!panel_ready) {
        return ESP_ERR_INVALID_STATE;
    }

    // Convert pixel counts into the last included coordinates
    int x_end = x + width - 1;
    int y_end = y + height - 1;
    // Panel addresses use inclusive endpoints, high byte first
    uint8_t columns[4] = { x >> 8, x & 0xFF, x_end >> 8, x_end & 0xFF };
    uint8_t pages[4] = { y >> 8, y & 0xFF, y_end >> 8, y_end & 0xFF };
    esp_err_t result = write_command_data(CASET, columns, sizeof(columns));
    if (result != ESP_OK) {
        return result;
    }
    result = write_command_data(PASET, pages, sizeof(pages));
    if (result != ESP_OK) {
        return result;
    }
    // Following data bytes now fill the selected window
    return write_command(RAMWR);
}

esp_err_t ili9488_fill_rect(int x, int y, int width, int height,
                          uint8_t red, uint8_t green, uint8_t blue)
{
    esp_err_t result = set_address_window(x, y, width, height);
    if (result != ESP_OK) {
        return result;
    }

    // Retain the upper six bits of each channel for RGB666
    for (int pixel = 0; pixel < width; pixel++) {
        row_buffer[pixel * 3] = red & 0xFC;
        row_buffer[pixel * 3 + 1] = green & 0xFC;
        row_buffer[pixel * 3 + 2] = blue & 0xFC;
    }
    // Reuse the same color row and send only the requested width
    for (int row = 0; row < height; row++) {
        result = write_data(row_buffer, (size_t)width * 3);
        if (result != ESP_OK) {
            return result;
        }
    }
    return ESP_OK;
}

esp_err_t ili9488_fill_screen(uint8_t red, uint8_t green, uint8_t blue)
{
    return ili9488_fill_rect(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, red, green, blue);
}

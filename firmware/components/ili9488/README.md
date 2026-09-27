# ILI9488 Display Driver

The driver configures SPI2 at 20 MHz in mode 0 and uses native 320 x 480 coordinates. It exposes separate bus and panel initialization, bounded rectangle drawing, full-screen fills, and packed RGB888 image drawing.

## Interface

| Function | Purpose |
|---|---|
| `ili9488_bus_init` | Configure DC/RESET and register the SPI device once |
| `ili9488_panel_init` | Reset/configure the LCD using the existing bus |
| `ili9488_panel_sleep` | Send display-off and sleep-in commands; invalidate drawing readiness |
| `ili9488_fill_rect` | Fill a fully contained rectangle; reject invalid bounds |
| `ili9488_fill_screen` | Fill the whole panel using the rectangle implementation |
| `ili9488_draw_rgb888` | Stream packed RGB888 rows into a bounded rectangle |

The driver returns errors to its caller. Negative coordinates, nonpositive dimensions, and out-of-bounds rectangles return `ESP_ERR_INVALID_ARG`. Valid drawing before panel initialization returns `ESP_ERR_INVALID_STATE`. Repeated bus initialization is rejected; repeated panel initialization reuses the connection.

Panel sleep sends `0x28` followed by `0x10`. It returns without waiting; the caller must allow at least 120 ms before processor sleep or panel reinitialization. Drawing remains blocked until a successful `ili9488_panel_init`, including when a sleep command fails. The power manager handles that interval while continuing button polling. Sleep commands do not switch off the directly powered backlight.

## Pixel Transfers

Colors are supplied as RGB888 channels and masked to their upper six bits for RGB666 transmission. A static DMA-capable 960-byte buffer holds one row; narrower rectangles send only their actual row length. No full-screen framebuffer is allocated. Image drawing requires exactly `width * height * 3` bytes and returns `ESP_ERR_INVALID_SIZE` for any other length. Null pixel pointers and invalid bounds are rejected before sending commands. Source pixels remain unchanged.

All calls must run in one task. Drawing is synchronous, so button sampling pauses during display initialization and diagnostics. The transition module returns to input sampling between bounded drawing steps. Panel reset and full recovery drawing remain synchronous.

## Startup Diagnostic

The application configures the bus, checks that drawing before panel initialization is rejected, and initializes the panel. When the optional startup diagnostic is enabled, it checks invalid rectangles and repeated bus initialization, displays black/red/green/blue/white fills, reinitializes the panel, and shows a corner pattern for five seconds. The first embedded slide is then drawn and button polling begins. Diagnostics default off in the slide firmware.

Native-coordinate markers:

- Red: (0, 0); green: (300, 0).
- Blue: (0, 460); white: (300, 460).
- Each marker is 20 x 20 pixels.
- Center: yellow 80 x 40 rectangle.
- Cyan horizontal line: y=120, full width.
- Magenta vertical line: x=80, full height.

Observed physical corner mapping (2026-09-22): top-left blue, top-right red, bottom-left white, bottom-right green. These positions map the native coordinates to the tested physical orientation.

## Validation

The firmware builds and the driver passes local simulated SPI/GPIO tests for configuration, initialization order, bounds, pixel data, and error propagation. The color sequence and corner positions were confirmed on the LCD. Edge rendering and repeated cold starts remain to be verified. MISO is disconnected, so successful transfers cannot confirm the panel's visible output.

After flashing, inspect each fill and pattern, confirm button logs resume, and repeat at least five full power cycles with LCD power on. Record actual results in `docs/testing.md`.

The reset delays and register values follow the working prototype: MADCTL `0x88`, COLMOD `0x66`, and conservative reset/sleep-out waits.

- [ILI9488 register and timing reference](https://files.waveshare.com/upload/2/2d/ILI9488_Data_Sheet.pdf)
- [ESP-IDF v6.1 SPI master API](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-reference/peripherals/spi_master.html)

# ILI9488 Display Driver

The driver configures SPI2 at 20 MHz in mode 0 and uses native 320 x 480 coordinates. It exposes separate bus and panel initialization, bounded rectangle drawing, and full-screen fills.

## Interface

| Function | Purpose |
|---|---|
| `ili9488_bus_init` | Configure DC/RESET and register the SPI device once |
| `ili9488_panel_init` | Reset/configure the LCD using the existing bus |
| `ili9488_fill_rect` | Fill a fully contained rectangle; reject invalid bounds |
| `ili9488_fill_screen` | Fill the whole panel using the rectangle implementation |

The driver returns errors to its caller. Negative coordinates, nonpositive dimensions, and out-of-bounds rectangles return `ESP_ERR_INVALID_ARG`. Valid drawing before panel initialization returns `ESP_ERR_INVALID_STATE`. Repeated bus initialization is rejected; repeated panel initialization reuses the connection.

## Pixel Transfers

Colors are supplied as RGB888 channels and masked to their upper six bits for RGB666 transmission. A static DMA-capable 960-byte buffer holds one row; narrower rectangles send only their actual row length. No full-screen framebuffer is allocated.

All calls must run in one task. Drawing is synchronous, so button sampling pauses during display initialization and diagnostics. Incremental rendering is a later stage.

## Startup Diagnostic

The application configures the bus, checks that drawing before panel initialization is rejected, initializes the panel, and runs the diagnostic module. It checks invalid rectangles and repeated bus initialization, displays black/red/green/blue/white fills, reinitializes the panel, and leaves a corner pattern visible. Button polling then resumes.

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

The starting reset delays and register values follow the supplied prototype: MADCTL `0x88`, COLMOD `0x66`, and conservative reset/sleep-out waits.

- [ILI9488 register and timing reference](https://files.waveshare.com/upload/2/2d/ILI9488_Data_Sheet.pdf)
- [ESP-IDF v6.1 SPI master API](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-reference/peripherals/spi_master.html)

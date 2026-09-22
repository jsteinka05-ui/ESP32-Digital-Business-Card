# Firmware

C firmware for the Seeed Studio XIAO ESP32-C3, developed with ESP-IDF v6.1.

## Current Behavior

At startup, the application initializes the LCD, validates both embedded images, and displays the business card. A short press advances on release; a second short press returns to the business card. The image renderer and navigation pass software checks; this version has been flashed to the board, and the revised QR appearance has been confirmed on the display. Repeated navigation, QR scanning, and cold-start checks remain open.

The button module configures GPIO6 as an input with an internal pull-up. The application samples it approximately every 10 ms and accepts transitions after 30 ms of stable input.

| Serial message | Meaning |
|---|---|
| `Button input ready` | GPIO initialization succeeded |
| `Button pressed` | A press passed the debounce interval |
| `Short press released` | A tracked press lasted less than 900 ms |
| `Long press released` | A tracked press lasted at least 900 ms |
| `Button released (no tracked press)` | A button held at startup was released without generating a gesture |

Durations are measured between accepted press and release transitions. A tracked release emits the short or long event instead of a separate release event. Holding never repeatedly emits a gesture. The first reading establishes state without generating a button event.

The button module returns events; the application logs them and draws the next slide on a short release. A long release currently only logs, leaving the slide unchanged. LCD recovery, animation, and sleep are later stages.

Drawing is synchronous and pauses button sampling during each full-screen transfer. A complete press during that interval can be missed. Input sampling during rendering will be addressed with incremental transitions. Draw failures are logged and leave the selected index unchanged; the screen may contain a partial image until a later successful redraw.

## Build, Flash, and Monitor

Install ESP-IDF v6.1 with the ESP32-C3 tools. In VS Code, open this `firmware` directory, select the installed ESP-IDF environment, and open an ESP-IDF terminal here.

```text
idf.py build
idf.py -p <PORT> flash monitor
```

Replace `<PORT>` with the board's serial port selected through `ESP-IDF: Select Port to Use`. On Windows this is a COM port; its number varies by computer. Close other serial monitors before flashing. Exit the monitor with Ctrl+].

The project selects the ESP32-C3 target. `sdkconfig.defaults` specifies 4 MB flash, the USB Serial/JTAG console, and the custom partition table. `partitions.csv` provides a 2 MB factory application partition for code and embedded images. The remaining flash is unallocated; OTA is not configured. Flashing replaces the firmware currently on the board.

Existing checkouts with a generated `sdkconfig` retain their old settings. In `idf.py menuconfig`, select **Partition Table > Custom partition table CSV** and use `partitions.csv`, then rebuild. Flash with `idf.py flash` so the updated partition table is written along with the application.

## Images and Diagnostics

Both source PNGs and generated RGB files are tracked in [images/](images/README.md). Conversion is a separate step, so building saved assets does not require Pillow. After editing artwork, regenerate the matching RGB file before building.

To run the earlier color and corner checks, enable **Business card > Run display diagnostics before showing slides** in `idf.py menuconfig`. The corner pattern remains visible for five seconds before the business-card slide appears. This option defaults off.

Each successful draw logs its duration in milliseconds, total free heap, minimum free heap, and the largest free internal 8-bit block. Record actual values from the serial monitor during hardware validation.

## Source Layout

- `main/main.c`: startup, current slide, polling, navigation, and draw/memory logging.
- `main/slides.c` and `main/slides.h`: flash image references, length validation, and indexed drawing.
- `main/button.h`: public button interface.
- `main/button.c`: GPIO configuration, raw readings, debouncing, and gesture classification.
- `main/display_diagnostics.c`: bounds checks, color fills, and the corner pattern.
- `components/ili9488/`: SPI transport, panel setup, and bounded drawing; see its [driver documentation](components/ili9488/README.md).

## Validation

The firmware builds with ESP-IDF v6.1. Simulated tests cover debounce timing, contact bounce, the 899/900/901 ms classification boundary, long holds, startup-held input, and reinitialization. The earlier button firmware was verified on the assembled board. The image renderer passes simulated transfer/error checks and the application passes simulated navigation checks. The Python converter tests run with `python -m unittest discover -s tools/tests -v`. Current slide firmware still requires hardware validation. See the [validation record](../docs/testing.md) for scope and results.

# Firmware

C firmware for the Seeed Studio XIAO ESP32-C3, developed with ESP-IDF v6.1.

## Current Behavior

At startup, the application initializes the LCD, validates both embedded images, and displays the business card. A short release advances with a black wipe and shuffled tile reveal. A long release reinitializes the LCD and redraws the selected slide without advancing it. The previous slide firmware and QR appearance were tested on the board; this recovery/animation version has been flashed and passed hardware checks for recovery, transitions, gesture suppression, QR scanning, and timing.

The button module configures GPIO6 as an input with an internal pull-up. The application accepts transitions after 30 ms of stable input. It samples once per loop, with a 10 ms task delay plus any drawing work. Animation steps are bounded; actual sampling gaps are logged for hardware measurement.

| Serial message | Meaning |
|---|---|
| `Button input ready` | GPIO initialization succeeded |
| `Button pressed` | A press passed the debounce interval |
| `Short press released` | A tracked press lasted less than 900 ms |
| `Long press released` | A tracked press lasted at least 900 ms |
| `Button released (no tracked press)` | A button held at startup was released without generating a gesture |

Durations are measured between accepted press and release transitions. A tracked release emits the short or long event instead of a separate release event. Holding never repeatedly emits a gesture. The first reading establishes state without generating a button event.

The button module returns events; the application controller decides whether to start a transition or recover the LCD. Accepted events during animation update the activity timestamp but never queue navigation or recovery. After startup, animation, or recovery, input must remain released for 30 ms before another gesture is accepted. After 300 seconds without accepted input, the power manager requests panel sleep and enters ordinary light sleep. Hardware sleep/wake validation is complete.

Normal transitions return to button sampling after one eight-row wipe strip or at most sixteen 8 x 8 tiles. The next step is eligible after 10 ms; a delayed loop never catches up with an unbounded batch. Each reveal covers all 2,400 tiles once. The selected slide changes only after the last tile succeeds.

Panel reset delays, startup drawing, and recovery drawing remain synchronous. A complete gesture during those operations can be missed; a held button is consumed until release. Drawing errors stop the transition and preserve the selected index. The screen may remain partial until a later successful transition or long-press recovery.

## Inactivity and Wake

The inactivity interval starts after the initial image is drawn. Accepted button events restart it, including events suppressed during animation. Sleep is deferred while a button is held, input is blocked, or rendering is active.

At timeout, the panel receives display-off and sleep-in commands. The application keeps polling for 120 ms while the panel settles. A press during that interval cancels sleep and restores the selected slide. Otherwise, the ESP32-C3 enters ordinary light sleep with active-low GPIO6 wake and its pull-up retained. Peripheral power-down sleep is not supported by this configuration.

Wake reinitializes the panel on the existing SPI bus and redraws the selected slide. The wake gesture is consumed until the button has remained released for 30 ms, even if held longer than 900 ms. The inactivity timer restarts after restoration. A rejected sleep attempt also restores the panel; errors are logged and the timeout restarts rather than retrying continuously.

The backlight remains powered because it is wired directly to the display supply. The manual LCD switch must remain on for button-only wake. Current and wake-latency checks passed on the assembled board; the validation record reports pass/fail results without numerical readings. The logged redraw time covers panel initialization and drawing, not the complete physical wake latency.

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

Startup logs the initial drawing duration. Each ended transition logs its elapsed time and longest observed input-sampling interval, including the final step and task delay. Heap readings are logged after startup and transitions. Recovery logs the restored slide. Hardware measurements belong in the validation record.

## Source Layout

- `main/main.c`: startup, input polling, and timing/memory logging.
- `main/app_controller.c` and `main/app_controller.h`: selected slide, gesture suppression, activity tracking, and LCD recovery.
- `main/power_manager.c` and `main/power_manager.h`: inactivity timeout, panel settling, and GPIO light-sleep wake.
- `main/transitions.c` and `main/transitions.h`: incremental wipe, shuffled tile ordering, and bounded drawing steps.
- `main/slides.c` and `main/slides.h`: flash image references, length validation, indexed drawing, and region extraction.
- `main/button.h`: public button interface.
- `main/button.c`: GPIO configuration, raw readings, debouncing, and gesture classification.
- `main/display_diagnostics.c`: bounds checks, color fills, and the corner pattern.
- `components/ili9488/`: SPI transport, panel setup, and bounded drawing; see its [driver documentation](components/ili9488/README.md).

## Validation

The firmware builds with ESP-IDF v6.1. Simulated tests cover debounce timing, contact bounce, the 899/900/901 ms classification boundary, long holds, startup-held input, and reinitialization. The earlier button firmware was verified on the assembled board. The image renderer passes simulated transfer/error checks and the application passes simulated navigation checks. The Python converter tests run with `python -m unittest discover -s tools/tests -v`. Integrated simulated checks use the actual button logic and cover recovery, suppressed gestures, complete tile coverage, final pixels, and failure handling. Recovery, repeated transitions, gesture suppression, QR scanning, and timing/memory checks also passed on the assembled board. Sleep tests cover timeout boundaries, entry cancellation, wake-gesture suppression, repeated restoration, and errors with simulated peripherals. Hardware sleep/wake, current, and wake-latency checks passed. See the [validation record](../docs/testing.md) for scope and results.

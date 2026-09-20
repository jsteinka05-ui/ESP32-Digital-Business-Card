# Firmware

Minimal C firmware for the Seeed Studio XIAO ESP32-C3, developed with ESP-IDF v6.1.

## Current Behavior

The button module configures GPIO6 as an input with an internal pull-up. The application samples it approximately every 10 ms and logs changes over the USB serial console:

- `Status: 1`: pressed.
- `Status: 0`: released.

The initial reading establishes the previous state without printing. Inputs are raw; debouncing and short/long-press classification are not implemented yet. The display and sleep features are later stages.

## Build, Flash, and Monitor

Install ESP-IDF v6.1 with the ESP32-C3 tools. In VS Code, open this `firmware` directory, select the installed ESP-IDF environment, and open an ESP-IDF terminal here.

```text
idf.py build
idf.py -p <PORT> flash monitor
```

Replace `<PORT>` with the board's serial port selected through `ESP-IDF: Select Port to Use`. On Windows this is a COM port; its number varies by computer. Close other serial monitors before flashing. Exit the monitor with Ctrl+].

The project selects the ESP32-C3 target. `sdkconfig.defaults` specifies 4 MB flash and the USB Serial/JTAG console. Flashing replaces the firmware currently on the board.

## Source Layout

- `main/main.c`: initialization, polling, and transition logging.
- `main/button.h`: public button interface.
- `main/button.c`: GPIO configuration and raw pressed-state reading.

## Validation

The earlier counter exercise was built, flashed, and observed over serial. The raw button checkpoint was tested on the assembled board: press/release changes were reported as expected, without repeated messages while held or idle. Debounce and timing boundary validation belong to the remaining F2 checkpoints.

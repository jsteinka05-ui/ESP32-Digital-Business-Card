# Firmware

Minimal C firmware for the Seeed Studio XIAO ESP32-C3, developed with ESP-IDF v6.1.

## Current Behavior

The button module configures GPIO6 as an input with an internal pull-up. The application samples it approximately every 10 ms and accepts transitions after 30 ms of stable input.

| Serial message | Meaning |
|---|---|
| `Button input ready` | GPIO initialization succeeded |
| `Button pressed` | A press passed the debounce interval |
| `Short press released` | A tracked press lasted less than 900 ms |
| `Long press released` | A tracked press lasted at least 900 ms |
| `Button released (no tracked press)` | A button held at startup was released without generating a gesture |

Durations are measured between accepted press and release transitions. A tracked release emits the short or long event instead of a separate release event. Holding never repeatedly emits a gesture. The first reading establishes state without generating a button event.

The button module returns events; the application currently logs them. Display navigation, LCD recovery, and sleep behavior are later stages.

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
- `main/button.c`: GPIO configuration, raw readings, debouncing, and gesture classification.

## Validation

The firmware builds with ESP-IDF v6.1. Simulated tests cover debounce timing, contact bounce, the 899/900/901 ms classification boundary, long holds, startup-held input, and reinitialization. Expected operation after flashing has been verified on the assembled board. See the [validation record](../docs/testing.md) for scope and results.

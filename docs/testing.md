# Firmware Validation

## Button Input : 2026-09-21

Environment: ESP-IDF v6.1, ESP32-C3 target, Seeed Studio XIAO ESP32-C3.

### Build

The application builds and links successfully with the GPIO and timer components. This verifies compilation; physical behavior is recorded separately below.

### Simulated Input

The current `button.c` was compiled for RISC-V and executed in QEMU with GPIO configuration and input stubs. Tests supplied explicit readings and timestamps to the actual button implementation. These tests exercise software logic, not electrical behavior or FreeRTOS scheduling. The test harness is local and is not included in this repository.

| Check | Result |
|---|---|
| Active-low reading and GPIO6 input configuration | Passed |
| Candidate rejected at 29,999 us and accepted at 30,000 us | Passed |
| Bounce restarts the stability interval on press and release | Passed |
| No repeated events while held or after a completed release | Passed |
| 899 ms press classified short | Passed |
| 900 ms and 901 ms presses classified long | Passed |
| Long hold produces its gesture only on release | Passed |
| Button held at startup releases without a short/long gesture | Passed |
| A normal gesture works after releasing the startup-held button | Passed |
| Reinitialization clears an unfinished gesture | Passed |

### Hardware

Expected button operation was verified after flashing the assembled board. Earlier raw-input testing established press/release detection and quiet output during steady hold and idle states. Exact timing-boundary results above come from simulated timestamps, not hand-timed physical presses.

### Remaining Validation

- Reset and USB power-cycle behavior from the application-foundation checklist.
- Display navigation and recovery after gesture events are connected to the display driver.
- Sleep/wake behavior and current measurements in the later power-management stage.

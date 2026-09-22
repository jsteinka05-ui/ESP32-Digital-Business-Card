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

## Display Driver : 2026-09-21

Status: code implemented; physical display validation remaining.

The complete application builds with ESP-IDF v6.1. A local RISC-V/QEMU harness executes the actual display driver with stubbed GPIO, SPI, and delays. This verifies software behavior without simulating the LCD's optical output or electrical timing. The harness remains local.

| Simulated check | Result |
|---|---|
| GPIO mapping, SPI mode/speed, DMA selection, and transfer limit | Passed |
| Reset levels/delays and ordered panel commands/parameters | Passed |
| Repeated bus initialization rejected; panel reinitialization reuses bus | Passed |
| Drawing before panel initialization rejected | Passed |
| Negative, zero, overflowing, and out-of-range rectangles produce no transfers | Passed |
| Last-pixel address bytes and inclusive endpoints | Passed |
| RGB666 channel masking and narrow-row byte counts | Passed |
| Full-screen fill sends 480 rows of 960 bytes | Passed |
| GPIO/SPI errors stop the operation and propagate to the caller | Passed |
| Failed panel initialization prevents subsequent drawing | Passed |
| Failed device registration releases the owned bus; failed cleanup blocks duplicate setup | Passed |

The display firmware was flashed and verified on COM4. On 2026-09-22, the color sequence was confirmed working and the physical corner positions were reported as follows:

| Physical corner | Marker |
|---|---|
| Top-left | Blue |
| Top-right | Red |
| Bottom-left | White |
| Bottom-right | Green |

The single yellow-pixel visual marker was subsequently removed. Simulated last-pixel bounds and byte-order checks remain valid. Edge rendering, five cold starts, and button logging after diagnostics still require confirmation. The marker removal has been built but not yet flashed.

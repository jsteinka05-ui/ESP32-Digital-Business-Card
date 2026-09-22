# Digital Business Card — Firmware Rewrite — Design Specification

## Overview

This project is a standalone digital business card built around a Seeed Studio XIAO ESP32-C3 and a 3.5-inch ILI9488 SPI TFT display. A working physical prototype with a 3D-printed enclosure has been completed, and a custom PCB is already implemented.

This document defines the architecture, intended behavior, and acceptance criteria for the firmware rewrite in C using ESP-IDF. The rewrite will preserve the device’s slide display, button navigation, animated transitions, and LCD recovery behavior while improving software organization and validation.

A five-minute inactivity timeout with low-power idle and button wake-up is a planned addition. Its implementation will include evaluating display sleep, backlight control, and processor sleep modes.

The physical prototype, wiring, and custom PCB assembly are complete. This specification covers the replacement firmware and the planned low-power feature.

## Project Goals

- Display contact information and a scannable LinkedIn QR code without a computer or network connection.
- Provide a simple interface using one physical button.
- Support animated transitions between slides.
- Recover the LCD after it is power-cycled independently of the microcontroller.
- Enter a low-power state after five minutes without button input.
- Restore the current slide when awakened.
- Maintain a modular and reproducible firmware project.
- Preserve the possibility of future Wi-Fi and Bluetooth Low Energy features.

Wireless functionality is outside the initial implementation scope.

## Hardware Selection

### Seeed Studio XIAO ESP32-C3

The XIAO ESP32-C3 combines a compact development board with sufficient processing capability, storage, and interfaces for this project.

Relevant specifications include:

- Approximately 21 × 17.8 mm board footprint.
- Single-core, 32-bit RISC-V processor operating at up to 160 MHz.
- 400 KB of on-chip SRAM.
- 4 MB of onboard flash.
- 2.4 GHz Wi-Fi.
- Bluetooth Low Energy.
- SPI and general-purpose input/output support.

The ESP32-C3 can be considered overkill for displaying two images and handling a button input. I chose it because I wanted the possibility of expanding the project through Wi-Fi and Bluetooth Low Energy without replacing the microcontroller.

Potential extensions include wireless content updates, configuration through a local web interface, and BLE-based interaction. The existing device operates offline, and the initial firmware rewrite will retain that behavior.

Its small footprint also limits the space occupied by the control electronics behind the display.

Reference: [Seeed Studio XIAO ESP32-C3 documentation](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)

### ILI9488-Based 3.5-Inch TFT Display

The display provides space for readable contact information, personal artwork, and a substantial QR code. Color output and animated transitions support the intended visual presentation.

The display is held in landscape orientation, with artwork designed at 480 × 320 pixels.

SPI reduces signal wiring compared with a parallel display interface. The tradeoff is limited transfer bandwidth, which affects full-screen updates and animation speed.

The TFT backlight also consumes power during viewing. This design prioritizes color and animation, with power management intended to reduce consumption when the device is inactive.

Reference: [Example ILI9488 SPI module documentation](https://lcdwiki.com/3.5inch_SPI_Module_ILI9488_SKU%3AMSP3520)

### Momentary Pushbutton

A single momentary button provides navigation and display recovery. The planned low-power feature will also use this button for wake-up.

The button connects GPIO6 to ground and uses the microcontroller’s internal pull-up. The input reads high when released and low when pressed.

This arrangement reduces external components and keeps the interface simple. The implemented input module requires 30 ms of unchanged input before accepting a transition. It samples approximately every 10 ms and measures gesture duration between accepted press and release transitions. A button held at startup must be released before a new gesture can be tracked.

### LCD Power Switch

The hardware arrangement includes a switch that controls LCD power while leaving the ESP32-C3 running.

Removing LCD power causes the display controller to lose its configuration. Restoring that power does not automatically restart the application or initialize the panel.

The device provides a long-press recovery action that resets the LCD and redraws the current slide. The replacement firmware will preserve this behavior.

This switch does not provide whole-device shutdown. A future hardware revision may change the power arrangement.

## Existing Prototype Wiring

The assembled PCB uses the following connections. This pin mapping is the hardware interface for the firmware rewrite.

| Signal | ESP32-C3 GPIO or connection |
|---|---|
| LCD SCK | GPIO8 |
| LCD MOSI / SDI | GPIO10 |
| LCD MISO / SDO | Disconnected |
| LCD CS | GPIO4 |
| LCD DC / RS | GPIO5 |
| LCD RESET | GPIO3 |
| Button | GPIO6 to ground |
| LCD VCC | 3.3 V through the LCD power arrangement |
| LCD backlight | 3.3 V display supply; no software switching |
| LCD ground | Common ground with the microcontroller |


## Software Platform

The replacement firmware will use C and ESP-IDF.

C supports direct work with memory, pointers, structures, and bit operations. ESP-IDF provides the GPIO, SPI, timing, FreeRTOS, and power-management facilities required by the design.

ESP-IDF also provides a path to future networking features without changing frameworks.

Local and automated builds will use the same ESP-IDF version.

## Repository Organization

The project separates firmware, hardware documentation, and design records:

```text
firmware/
    main/
    components/ili9488/
    images/
    tools/
hardware/
    schematic/
    manufacturing/
    enclosure/
docs/
.github/
    workflows/
```

The firmware directory will contain the ESP-IDF build configuration. Hardware files will describe the physical assembly, while `docs/` will contain design decisions, testing procedures, and development notes.

## Software Architecture

| Module | Responsibility |
|---|---|
| Application | Startup, current slide, operating state, and event handling |
| Button | Debouncing, duration measurement, and input events |
| Display driver | SPI communication, panel initialization, and drawing |
| Slides | Embedded image references and slide metadata |
| Transitions | Animated changes between slides |
| Power management | Inactivity tracking, sleep entry, and wake restoration |
| Image conversion tool | Conversion of source artwork into display-ready bytes |

Modules will be introduced as functionality develops.

Only one execution context will control the display at a time. Drawing, recovery, and sleep operations must not issue overlapping panel commands.

Display communication setup and panel initialization will be separate operations. Startup requires both; LCD recovery should reuse the existing SPI connection.

## Image Storage and Rendering

### Embedded Assets

The replacement firmware will retain the two-image embedded storage approach used by the existing device.

For a small, fixed slide set, this avoids an SD card, additional storage wiring, and a runtime filesystem.

The tradeoffs are that artwork changes require rebuilding and flashing, and raw images occupy significant flash space.

### Pixel Format and Memory

Each full-screen RGB image contains:

320 × 480 × 3 = 460,800 bytes

Two images require 921,600 bytes before application overhead.

Assets will use RGB888 storage: one byte each for red, green, and blue. The display will be configured for RGB666 transmission, retaining the upper six bits of each channel while sending three bytes per pixel.

One complete image exceeds the microcontroller’s total internal SRAM capacity. The renderer will therefore stream data from flash through small reusable buffers.

Image lengths and drawing bounds are validated before transmission. The current renderer streams one row at a time through a shared 960-byte buffer and leaves the source image unchanged. QR recoloring uses an additional 960-byte stack row to blend a translucent white panel over the business card's gold background while preserving the black modules.

Both RGB assets are embedded in the application flash image. A 2 MB factory application partition accommodates the 921,600 bytes of artwork plus firmware; the remaining space in the 4 MB flash is unallocated. OTA partitions are outside the current design.

### Orientation

The replacement renderer will initially retain native 320 × 480 panel coordinates with pre-rotated landscape artwork:

```text
480 × 320 landscape artwork
    → rotate counter-clockwise
    → store 320 × 480 RGB bytes
    → stream to the display
```

The converter requires exactly 480 × 320 source pixels and performs a lossless quarter-turn without resizing. Transparent artwork is composited onto white. Source PNGs and converted RGB assets are tracked together.

Color fills and corner markers established the mounted orientation: blue top-left, red top-right, white bottom-left, and green bottom-right. The revised QR appearance has been confirmed on the display. Physical QR scanning remains part of acceptance testing.

### SPI Performance

The replacement firmware will initially retain SPI mode 0 at 20 MHz, with operation revalidated on the target hardware.

At that rate, a complete image requires approximately 184 ms of pixel transfer alone:

460,800 × 8 ÷ 20,000,000 ≈ 0.184 seconds

Command transfers and software overhead increase the actual duration. Animations will account for this bandwidth limit.

## User Interaction

The replacement firmware preserves slide navigation and LCD recovery while adding inactivity tracking and defined input handling during animation.

### Startup

The application will:

1. Configure the button.
2. Initialize SPI and the LCD.
3. Display the business-card slide.
4. Begin processing input and tracking inactivity.

Panel reset and initialization delays will follow the applicable hardware requirements and be validated through repeated startup tests.

### Short Press

A debounced press lasting less than 900 ms will advance to the next slide upon release.

Navigation will wrap from the final slide to the first slide.

Normal slide changes will not reset the LCD.

### Long Press

A debounced press lasting at least 900 ms will trigger LCD recovery upon release.

Recovery will reset and configure the panel, redraw the current slide, and leave the slide index unchanged.

Each long press must produce exactly one recovery action and no navigation action.

### Input During Animation

Button input during an animation will update inactivity tracking but will not queue additional slide changes.

A gesture beginning during an animation must be released before another navigation gesture is accepted. This avoids unexpected actions when the animation finishes.

## Low-Power Idle State

Low-power idle is a planned addition to the firmware rewrite.

### Inactivity Detection

The device will enter a low-power idle state after 300 seconds without valid button input.

The timer begins after the initial slide is displayed. Each debounced press or release updates the activity timestamp.

Sleep entry will be deferred while:

- The button is held.
- An animation is running.
- Display recovery is in progress.

### Sleep Entry

When the inactivity timeout expires, the application will:

1. Complete any active display operation.
2. Preserve the current slide selection.
3. Place the display into an appropriate low-power condition.
4. Disable the backlight if supported by the hardware.
5. Enter an appropriate ESP32-C3 sleep mode with button wake-up enabled.

The display commands and processor sleep mode will be selected during implementation and verified on the assembled device.

### Backlight Control

The existing backlight connection has no software-controlled switching.

The manual LCD power switch removes display and backlight power. The processor cannot restore a mechanically switched supply. Automatic backlight shutdown requires a controllable power path; display sleep alone does not provide that control.

### Wake-Up

Pressing the button while idle will wake the device and restore the previously displayed slide.

This button-only behavior assumes the LCD supply switch remains on. If the LCD has been manually switched off, restore its supply before waking the device. If the processor is already awake when LCD power is restored, use long-press recovery. Wake restoration must reinitialize the panel when its state has been lost.

The wake-up gesture will be consumed entirely as a wake action. It will not advance the slide or trigger recovery, regardless of how long it remains held.

The application will wait for a stable release before accepting another gesture and restart the inactivity timer.

### Sleep-Mode Evaluation

Ordinary light sleep is the initial approach because it retains application state and supports the existing GPIO6 button with the GPIO peripheral powered.

ESP32-C3 GPIO wake from deep sleep is limited to GPIO0–5. GPIO6 therefore does not support the planned button wake in deep sleep. Peripheral power-down light-sleep configurations have the same restricted wake-pin requirement and are outside the initial implementation.

The implementation will evaluate ordinary light-sleep consumption, panel restoration, and wake latency on the assembled board. See [ESP-IDF sleep modes](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32c3/api-reference/system/sleep_modes.html) and the [ESP32-C3 GPIO capability definitions](https://github.com/espressif/esp-idf/blob/v6.0.1/components/soc/esp32c3/include/soc/soc_caps.h).

Active current, idle current, and wake-up latency are the power-management performance metrics.

## QR-Code Presentation

The QR slide will link to the LinkedIn profile and include a clear description of its purpose.

Scanning will be tested on the physical screen under documented viewing conditions.

## Validation Plan

The following acceptance checks define completion of the replacement firmware on the assembled hardware.

| Area | Acceptance check |
|---|---|
| Startup | Repeated full power cycles display the initial slide |
| Short press | One deliberate press advances exactly one slide |
| Long press | Recovery occurs once without changing slides |
| Debouncing | Contact bounce does not generate extra actions |
| Orientation | Corners and color patches appear correctly |
| Rendering | Invalid sizes and drawing bounds are rejected |
| Animation | Each transition finishes with the correct image |
| LCD recovery | Recovery restores the slide after LCD-only power cycling |
| QR code | The displayed code scans reliably |
| Inactivity | Idle begins after approximately 300 seconds without input |
| Timer reset | Valid input restarts the inactivity interval |
| Held button | The device does not sleep while the button is held |
| Wake-up | The previous slide returns without unintended navigation |
| Repeated sleep | Multiple sleep/wake cycles remain reliable |
| Power | Active and idle consumption are measured consistently |
| Reproducibility | A fresh checkout builds using the documented toolchain |

## Development Milestones

### Existing Hardware and Prototype Work

- Working physical prototype completed, including slide display, navigation, transitions, and LCD recovery.
- 3D-printed enclosure completed.
- Prototype wiring and custom PCB assembly completed; the PCB is implemented in the working device.

### Firmware Rewrite and Remaining Integration

The [firmware development plan](firmware-plan.md) groups the following work into stages F1–F7, corresponding to repository Steps 2–8.

1. Establish repository structure and hardware documentation.
2. Build minimal firmware with startup logging.
3. Implement debounced button input and press classification.
4. Initialize the LCD and display solid colors.
5. Implement bounded drawing operations.
6. Convert, embed, and display one image.
7. Add the second slide and navigation.
8. Implement independent LCD recovery.
9. Add animated transitions.
10. Evaluate backlight control and processor sleep.
11. Implement inactivity detection and wake-up.
12. Verify replacement-firmware operation on the completed custom PCB assembly.
13. Measure performance and document the completed device.

## Future Expansion

Possible extensions include wireless configuration, Wi-Fi content updates, BLE interaction, additional slides, and further PCB or enclosure revisions.

These features remain outside the firmware rewrite’s initial scope. The hardware selection leaves room to explore them after the replacement firmware and planned power-management feature are complete.

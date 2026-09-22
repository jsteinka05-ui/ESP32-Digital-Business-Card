# Firmware Development Plan

## Current Progress

The hardware baseline is complete: assembled PCB, wiring, enclosure, and operating prototype. The ESP-IDF v6.1 application now implements 30 ms button debouncing and release-based short/long-press classification. The firmware builds, simulated input tests pass, and operation after flashing has been verified on the board. F2 is complete. F3 display code and diagnostics are implemented and build successfully; the color sequence and mounted corner positions have been verified. Edge rendering, repeated cold starts, and button logging after diagnostics remain to be checked. The separate reset/power-cycle check from F1 remains open.

Development proceeds through seven firmware stages. Each stage produces a working increment, validation evidence, and a focused commit. Firmware stage F1 corresponds to Step 2 in the [progress log](progress.md); Step 1 records the hardware and repository baseline.

## Milestones

| Stage | Repository step | Deliverable | Completion criterion | Status |
|---|---|---|---|---|
| F1 | 2 | Minimal ESP-IDF application | Builds for ESP32-C3, logs periodically, and restarts correctly | Core working; final checks remaining |
| F2 | 3 | Button input and gesture events | Debounced short/long events occur once per gesture, classified on release | Complete |
| F3 | 4 | Display driver and bounded drawing | Repeated startup, color fills, corner markers, and rectangle bounds pass | Implemented; hardware validation remaining |
| F4 | 5 | Embedded images and slide navigation | Two correctly oriented slides cycle reliably and the displayed QR scans | Planned |
| F5 | 6 | LCD recovery and incremental animation | Recovery preserves the slide; transitions maintain input tracking | Planned |
| F6 | 7 | Inactivity and light sleep | A 300-second timeout enters sleep; button wake restores the slide without navigation | Planned |
| F7 | 8 | Release validation | Clean-checkout build, repeatable hardware tests, and measured results | Planned |

## Implementation Approach

- Use C with ESP-IDF and pin the tested framework version at F1.
- Keep application decisions separate from GPIO sampling and display transfers.
- Begin with one application task and synchronous display operations. At F5, advance animations in bounded steps between input samples.
- Store images in flash and stream them through a reusable transfer buffer.
- Keep panel reset/configuration separate from SPI bus creation.
- Preserve the existing native portrait image layout, GPIO mapping, and initial 20 MHz SPI configuration.
- Introduce modules and automated checks alongside the behavior they support.

## F1 — Application Foundation

Establish the root and main-component build files under `firmware/`, implement startup and periodic logging, and select the ESP32-C3 target. Record the tested ESP-IDF version and a repeatable build/flash workflow. No LCD or button code is required for this milestone.

Evidence: successful build, serial output, and reset/power-cycle behavior.

## F2 — Input Handling

Implement stable press/release detection and duration-based gesture events. A short press is less than 900 ms; a long press is at least 900 ms. Classification occurs on release. Separate the timestamp-driven input logic from the hardware read so boundary and bounce cases can be exercised deterministically.

Evidence: button test cases, event counts, and synthetic tests around the classification threshold.

## F3 — Display Driver

Add the ILI9488 component with command/data transfers, panel initialization, address windows, and bounded rectangle drawing. Establish color order and orientation with a diagnostic pattern before displaying artwork.

Evidence: repeatable cold starts, correct corner/color pattern, and invalid-coordinate rejection.

## F4 — Images and Navigation

Add the image converter, source artwork, embedded RGB assets, and slide metadata. Stream through a small buffer; reject incorrect image lengths. Integrate short-press navigation with immediate redraws before introducing animation.

Evidence: conversion checks, flash/heap measurements, correctly oriented slides, and a physical QR scan.

## F5 — Recovery and Animation

First implement long-press panel recovery without recreating the SPI bus. Then add the wipe and tile transition as incremental operations with input sampling between steps. Gestures during an animation update activity but do not queue navigation; release is required before accepting another gesture.

Evidence: independent LCD power-cycle recovery, repeated transitions, and defined behavior for presses during animation.

## F6 — Inactivity and Sleep

Implement the inactivity state machine before enabling sleep. Use ordinary light sleep with GPIO6 wake-up and the GPIO peripheral retained. ESP32-C3 deep-sleep GPIO wake is limited to GPIO0–5, so it is not the initial approach for the existing button connection.

The final timeout is 300 seconds. Wake consumes the entire initiating gesture and restores the current slide. Sleep is deferred during a held button, drawing, or recovery.

The current backlight remains connected to the display supply. Processor/display sleep and automatic backlight power switching are separate capabilities. Button-only wake assumes the manual LCD switch remains on.

Evidence: full-duration timeout tests, repeated wake cycles, wake latency, and whole-device active/idle current measurements.

## F7 — Release Validation

Run the acceptance matrix in the [design specification](design.md#validation-plan). Build from a clean checkout using the recorded toolchain and add an automated build check. Publish actual test results, resource measurements, build instructions, and a device demonstration.

Evidence: clean-checkout and automated builds, hardware results tied to a firmware revision, and the release demonstration.

## Commit and Documentation Policy

Each implementation commit includes its relevant documentation and test changes. Milestones may span multiple commits when they contain independently verifiable behavior, particularly input handling, display bring-up, recovery, and sleep.

The [progress log](progress.md) records completed stages and commit references. `firmware/README.md` contains the build instructions; [docs/testing.md](testing.md) records build, simulated-input, and hardware validation results. A successful build establishes compilation, while hardware behavior is recorded separately.

## Technical References

- [ESP-IDF getting started](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/get-started/index.html)
- [Build system](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-guides/build-system.html)
- [GPIO](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-reference/peripherals/gpio.html)
- [SPI master](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-reference/peripherals/spi_master.html)
- [Sleep modes](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-reference/system/sleep_modes.html)
- [ESP32-C3 GPIO capabilities](https://github.com/espressif/esp-idf/blob/v6.1/components/soc/esp32c3/include/soc/soc_caps.h)

The rewrite uses ESP-IDF v6.1; the references above follow that version.

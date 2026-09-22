# Progress and Commit Log

This log tracks the firmware rewrite and repository milestones.

## Completed Hardware

- Working contact-information and QR-code display.
- Button-controlled navigation, animated transitions, and LCD recovery.
- Custom PCB assembly and wiring.
- Printed enclosure with top, bottom, and button pieces.

## Step 1 / Commit 1 : Hardware and Design Baseline

Date: 2026-09-20

Milestone status: complete.

Commit title: `Add hardware designs, enclosure models, and firmware rewrite plan`

Commit: `7a0ee8d`.

### Deliverables

- Repository structure for firmware, hardware, and documentation.
- Ignore rules for generated files and local development settings.
- Firmware architecture, interaction requirements, and validation plan.
- KiCad schematic, board layout, and project settings.
- Manufacturing exports for hardware revisions 1.2, 1.3, and 1.4.
- Project-local XIAO libraries and the custom through-hole footprint.
- Top-case, bottom-case, and button STL models.
- Component inventory and upstream library attribution.

## Step 2 : Minimal ESP-IDF Application

Status: core application working

- [x] Set up ESP-IDF v6.1 and the ESP32-C3 development environment.
- [x] Establish the project build configuration.
- [x] Implement an increasing counter logged once per second, with a FreeRTOS task delay between messages.
- [x] Build, flash, and verify counter output on the device through the serial monitor.
- [x] Add a one-time startup log message before the loop.
- [ ] Verify counter restart after reset and a USB power cycle.
- [x] Record build, flash, and monitor instructions in `firmware/README.md`.

## Step 3 : Button Input: Checkpoint 1

Status: raw GPIO input checkpoint complete

Commit title: `Add GPIO6 button module and raw input polling`

Commit: `e0f1294`.

- [x] Separate the button interface and implementation into `button.h` and `button.c`.
- [x] Configure GPIO6 as an input with its internal pull-up enabled.
- [x] Expose the active-low input as a boolean pressed state.
- [x] Check initialization and sample the button approximately every 10 ms.
- [x] Log only changes, using `Status: 1` for pressed and `Status: 0` for released.
- [x] Verify expected press/release reporting on the assembled board, with no repeated messages during a steady hold or idle state.

This checkpoint established raw input polling. Debouncing and release-based classification are implemented in the completed checkpoint below.

## Step 3 : Button Input: Debouncing and Gestures

Date: 2026-09-21

Status: complete

Commit title: `Add button debouncing and release-based gesture detection`

Commit: `04a4c53`.

- [x] Accept press and release transitions after 30 ms of stable input.
- [x] Classify presses shorter than 900 ms as short and presses of at least 900 ms as long.
- [x] Emit one gesture event on release, with no repeated gestures while held.
- [x] Ignore gesture classification for a button already held at startup.
- [x] Log startup, accepted presses, and classified releases through the serial monitor.
- [x] Pass simulated bounce, threshold, startup-held, and reinitialization checks.
- [x] Build with ESP-IDF v6.1 and verify expected operation after flashing the board.

Validation results are recorded in [testing.md](testing.md). Display actions are the next stage; the current gesture events produce serial logs.

## Step 4 : Display Driver and Diagnostics

Date: 2026-09-22

Status: implemented; hardware validation remaining

- [x] Configure SPI2 and LCD control pins as a separate component.
- [x] Separate bus setup from repeatable panel initialization.
- [x] Implement RGB666 color fills and bounded rectangle drawing.
- [x] Add startup color, corner, line, and invalid-coordinate diagnostics.
- [x] Build successfully and pass simulated transfer/error-path checks.
- [x] Verify the color sequence and record physical corner positions: blue top-left, red top-right, white bottom-left, green bottom-right.
- [ ] Verify edge drawing on the LCD.
- [ ] Verify at least five cold starts and button logging after diagnostics.

## Upcoming Milestones

| Step | Scope | Status |
|---|---|---|
| 3 | Button input, debouncing, and press classification | Complete |
| 4 | Display initialization, drawing, and orientation | Implemented; hardware validation remaining |
| 5 | Embedded images and slide navigation | Planned |
| 6 | LCD recovery and incremental animated transitions | Planned |
| 7 | Inactivity timeout, light sleep, and button wake-up | Planned |
| 8 | Reproducible builds, measurements, and release validation | Planned |

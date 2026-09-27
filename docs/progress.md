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

Status: complete

- [x] Set up ESP-IDF v6.1 and the ESP32-C3 development environment.
- [x] Establish the project build configuration.
- [x] Implement an increasing counter logged once per second, with a FreeRTOS task delay between messages.
- [x] Build, flash, and verify counter output on the device through the serial monitor.
- [x] Add a one-time startup log message before the loop.
- [x] Verify restart after reset and a USB power cycle.
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

Validation results are recorded in [testing.md](testing.md). This checkpoint established gesture logging before display integration.

## Step 4 : Display Driver and Diagnostics

Date: 2026-09-22

Commit title: `Add ILI9488 display driver and startup diagnostics`

Commit: `632c107`.

Status: complete

- [x] Configure SPI2 and LCD control pins as a separate component.
- [x] Separate bus setup from repeatable panel initialization.
- [x] Implement RGB666 color fills and bounded rectangle drawing.
- [x] Add startup color, corner, line, and invalid-coordinate diagnostics.
- [x] Build successfully and pass simulated transfer/error-path checks.
- [x] Verify the color sequence and record physical corner positions: blue top-left, red top-right, white bottom-left, green bottom-right.
- [x] Verify edge drawing on the LCD.
- [x] Verify at least five cold starts and button logging after diagnostics.

## Step 5 : Embedded Images and Slide Navigation

Date: 2026-09-22

Commit title: `Add embedded slides and short-press navigation`

Commit: `876373f`.

Status: complete

- [x] Add the original business-card and QR artwork with reproducible RGB assets.
- [x] Convert 480 x 320 artwork to native 320 x 480 RGB888 without resizing.
- [x] Embed both slides in flash and validate image lengths before drawing.
- [x] Stream RGB666 pixels through the existing 960-byte row buffer.
- [x] Show the business card at startup and wrap navigation on short releases.
- [x] Retain the current slide index if a draw fails.
- [x] Add a 2 MB application partition and optional startup diagnostics.
- [x] Pass converter, rendering, and simulated navigation checks.
- [x] Build with ESP-IDF v6.1, including a fresh configuration with diagnostics enabled.
- [x] Flash the application and updated partition table with verified data hashes.
- [x] Match the QR background to the business card and verify its translucent panel appearance on the display.
- [x] Verify readable artwork, correct orientation, and one slide change per short press.
- [x] Verify at least 20 slide changes, wraparound, and no navigation on long holds.
- [x] Scan the displayed QR code and confirm its destination.
- [x] Verify five cold starts.
- [x] Record numerical draw-time and heap results in the release validation record.

## Step 6 : LCD Recovery and Transitions

Date: 2026-09-22

Status: complete

- [x] Recover the LCD on long release without recreating the SPI bus.
- [x] Redraw the selected slide without advancing it during recovery.
- [x] Add incremental black wipe and shuffled 8 x 8 tile reveal.
- [x] Preserve the QR background and translucent panel in every tile.
- [x] Sample input between bounded animation steps and record accepted activity.
- [x] Discard gestures during drawing and require a stable release before rearming.
- [x] Preserve the selected slide and stop animation on transfer failure.
- [x] Pass integrated checks with the actual button, controller, transition, and slide code.
- [x] Verify 20 simulated transitions with complete tile coverage and expected final pixels.
- [x] Build with ESP-IDF v6.1.
- [x] Flash the recovery and transition firmware with verified data hashes.
- [x] Verify LCD-only power-cycle recovery on both slides.
- [x] Verify 20 transitions and presses held across the animation boundary on hardware.
- [x] Check transition timing, input-sampling gaps, and heap readings on hardware.
- [x] Confirm QR scanning after transition and recovery.

## Step 7 : Inactivity Sleep and Button Wake

Date: 2026-09-22

Commit title: `Add LCD recovery, transitions, and inactivity sleep`

Status: complete

Hardware validation completed: 2026-09-27

- [x] Start the 300-second timeout after drawing and reset it on accepted input.
- [x] Defer sleep during held input, rendering, and recovery.
- [x] Add panel sleep with 120 ms settling and input cancellation.
- [x] Configure ordinary light sleep with GPIO6 wake and retained pull-up.
- [x] Restore the selected slide using the existing SPI bus.
- [x] Consume the wake gesture until a stable release.
- [x] Restart inactivity after wake, cancellation, or failed restoration.
- [x] Pass timeout, cancellation, error-path, and ten simulated sleep/wake cycles.
- [x] Build with ESP-IDF v6.1 with diagnostics disabled and enabled.
- [x] Flash the sleep/wake firmware with verified data hashes.
- [x] Verify the full five-minute timeout on hardware.
- [x] Verify ten sleep/wake cycles on both slides and long-held wake suppression.
- [x] Verify sleep deferral and cancellation during button activity.
Current consumption and physical wake latency are outside this release's measurement scope.

## Step 8 : Release Validation

Status: in progress

- [x] Complete the remaining functional hardware acceptance checks.
- [x] Confirm the clean-checkout build workflow.
- [x] Record startup drawing, transition timing, sampling gaps, and heap measurements.
- [x] Add device photographs and a demonstration.

## Upcoming Milestones

| Step | Scope | Status |
|---|---|---|
| 3 | Button input, debouncing, and press classification | Complete |
| 4 | Display initialization, drawing, and orientation | Complete |
| 5 | Embedded images and slide navigation | Complete |
| 6 | LCD recovery and incremental animated transitions | Complete |
| 7 | Inactivity timeout, light sleep, and button wake-up | Complete |
| 8 | Reproducible builds, measurements, and release validation | In progress |

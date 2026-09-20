# Progress and Commit Log

This log tracks the firmware rewrite and repository milestones. The physical prototype was completed before this development phase.

## Completed Hardware

- Working contact-information and QR-code display.
- Button-controlled navigation, animated transitions, and LCD recovery.
- Custom PCB assembly and wiring.
- Printed enclosure with top, bottom, and button pieces.

## Step 1 / Commit 1 — Hardware and Design Baseline

Date: 2026-09-20

Milestone status: complete.

Commit title: `Add hardware designs, enclosure models, and firmware rewrite plan`

### Deliverables

- Repository structure for firmware, hardware, and documentation.
- Ignore rules for generated files and local development settings.
- Firmware architecture, interaction requirements, and validation plan.
- KiCad schematic, board layout, and project settings.
- Manufacturing exports for hardware revisions 1.2, 1.3, and 1.4.
- Project-local XIAO libraries and the custom through-hole footprint.
- Top-case, bottom-case, and button STL models.
- Component inventory and upstream library attribution.

### Repository Checks

- Hardware and enclosure files matched their source copies.
- XIAO library paths resolved within the project.
- Symbol pin definitions and custom footprint pad geometry matched the design.
- Local documentation links and Markdown formatting checked.

## Step 2 — Minimal ESP-IDF Application

Status: planned.

- Set up the ESP32-C3 development environment.
- Establish the project build configuration.
- Implement startup logging.
- Build, flash, and verify serial output on the device.

## Upcoming Milestones

| Step | Scope | Status |
|---|---|---|
| 3 | Button input, debouncing, and press classification | Planned |
| 4 | Display initialization, drawing, and orientation | Planned |
| 5 | Embedded images and slide navigation | Planned |
| 6 | LCD recovery and animated transitions | Planned |
| 7 | Inactivity timeout, sleep, and button wake-up | Planned |
| 8 | Performance measurements and final firmware validation | Planned |

The current baseline contains hardware and design documentation. Firmware implementation begins with Step 2. Commit history provides the version record for each milestone.

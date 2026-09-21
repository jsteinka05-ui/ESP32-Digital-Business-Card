# ESP32 Digital Business Card

A standalone digital business card built around a **Seeed Studio XIAO ESP32-C3** and a **3.5-inch ILI9488 SPI TFT display**. This repository documents its hardware and a firmware rewrite in **C using ESP-IDF**.

The device is designed to display personal contact information and a LinkedIn QR code, with a single button for slide navigation, display recovery, and wake-up.

## Project Status

A working physical prototype, custom PCB assembly, wiring, and 3D-printed enclosure are complete. Current development focuses on the firmware rewrite and a new inactivity sleep feature.

The physical prototype predates this repository. The C firmware rewrite now includes a minimal ESP-IDF v6.1 application that builds, flashes, and polls GPIO6 approximately every 10 ms on the ESP32-C3, debounces input over 30 ms, and classifies short and long presses on release. Display operation and sleep support are upcoming firmware stages.

Hardware design files and printable enclosure models are available under [hardware/](hardware/README.md).

## Existing Prototype Features

- Business-card and LinkedIn QR-code slides.
- Standalone operation with images embedded in firmware; no network connection or SD card required.
- Animated transitions controlled by a physical button.
- Long-press recovery after the LCD is power-cycled independently of the microcontroller.

## Planned Additions

- Low-power idle after five minutes without button input.
- Button wake-up that restores the current slide without advancing it.

The current backlight is powered directly from the display supply. Automatic backlight shutdown would require a hardware change. Wi-Fi and Bluetooth Low Energy are future expansion options.

## Hardware and Design Choices

| Part | Purpose |
|---|---|
| Seeed Studio XIAO ESP32-C3 | Compact controller with flash storage, SPI, and room for wireless expansion |
| 3.5-inch ILI9488 SPI TFT | Color contact-information and QR-code display |
| Momentary pushbutton | Navigation, display recovery, and wake-up |
| LCD power switch | Controls LCD power independently of the microcontroller |

The ESP32-C3 can be considered overkill for displaying two images and reading a button. I chose it to leave room for future Wi-Fi and Bluetooth Low Energy functionality without replacing the controller.

Embedded images keep the initial storage design simple, while SPI limits the display wiring. The tradeoffs include flash usage, display-update bandwidth, and the need to rebuild firmware when artwork changes.

See the [design document](docs/design.md) for component rationale, wiring, memory calculations, and power-management considerations.

## Intended Controls

The firmware rewrite targets the following controls, including the planned sleep and wake behavior:

| Input or condition | Intended behavior |
|---|---|
| Short press: less than 900 ms | Advance to the next slide upon release |
| Long press: at least 900 ms | Reinitialize the LCD and redraw the current slide upon release |
| Five minutes without button input | Enter a low-power idle state |
| Button press while idle | Wake and restore the current slide; consume the gesture without navigation |

The LCD switch leaves the ESP32-C3 running. Display recovery is needed because restoring LCD power does not automatically repeat the panel initialization sequence.

Button-only wake-up assumes the LCD supply switch remains on. If the LCD is manually switched off, restore its power before waking the device or requesting display recovery.

## Repository Layout

```text
firmware/                 ESP-IDF application and build configuration
    main/                 Application logic
    components/ili9488/   Display driver
    images/               Source artwork and display assets
    tools/                Image conversion utilities
hardware/                 Hardware documentation
    schematic/            KiCad project, PCB, schematic, and local libraries
    manufacturing/        Versioned Gerber and drill packages
    enclosure/            Printable STL models and enclosure notes
docs/                     Design decisions and testing documentation
.github/workflows/        Future automated build checks
```

## Development Setup

The intended development environment is:

- VS Code with Espressif’s ESP-IDF extension.
- ESP-IDF with the ESP32-C3 target and its toolchain.
- A USB data connection to the XIAO ESP32-C3.
- Python for image-conversion tooling when added.

Firmware development takes place under `firmware/`. The application now runs on the board with debounced button input and gesture logging; the [progress log](docs/progress.md) tracks the remaining F1 checks and upcoming stages.

## Roadmap

- [x] Establish repository structure and ignore rules.
- [x] Document the initial design.
- [x] Complete prototype wiring, custom PCB assembly, and printed enclosure.
- [x] Organize KiCad files, manufacturing exports, and enclosure models.
- [x] Implement startup logging and debounced button input.
- [ ] Initialize the display and verify drawing, colors, and orientation.
- [ ] Display embedded slides and implement navigation.
- [ ] Add LCD recovery and animated transitions.
- [ ] Implement and measure inactivity sleep and button wake-up.
- [ ] Publish build instructions, test results, and a device demonstration.

## Documentation

- [Firmware rewrite design](docs/design.md): architecture, hardware selection, interaction rules, and validation plan.
- [Firmware validation](docs/testing.md): build checks, simulated button tests, and hardware results.
- [Firmware development plan](docs/firmware-plan.md): implementation stages and completion criteria.
- [Hardware](hardware/README.md): KiCad project, parts inventory, and manufacturing revisions.
- [Enclosure](hardware/enclosure/README.md): top case, bottom case, and button models.
- [Progress and commit log](docs/progress.md): milestones, checks, and commit references.

## License

Third-party library sources and licensing are listed in the [library documentation](hardware/schematic/libraries/README.md), with the upstream license included alongside its files.

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

The single yellow-pixel visual marker was subsequently removed. Simulated last-pixel bounds and byte-order checks remain valid. Edge rendering, five cold starts, and button logging after diagnostics still require confirmation. The marker removal is included in the flashed slide firmware; startup diagnostics are disabled by default.

## Embedded Images and Navigation : 2026-09-22

Status: software checks passed; hardware validation pending.

### Build and Storage

The default ESP-IDF v6.1 build produces a 1,109,792-byte application in a 2,097,152-byte factory partition, leaving 987,360 bytes free. Both image symbols reside in `.flash.rodata`, each spanning exactly 460,800 bytes. The renderer reuses the existing 960-byte DRAM buffer. QR recoloring adds a 960-byte stack row; neither path allocates a full-screen framebuffer.

These are build-time measurements. Runtime free heap, minimum heap, largest free internal block, and draw duration are logged after successful draws but have not yet been measured on hardware for this version.

A separate build from fresh generated configuration also passed with startup diagnostics enabled. It selected the custom partition table from defaults. Normal operation was built with diagnostics disabled.

### Software Checks

| Check | Result |
|---|---|
| Converter rotation, RGB channel order, and exact output length | Passed |
| Invalid source dimensions rejected without changing existing output | Passed |
| Transparent pixels composited onto white | Passed |
| Source file protected from being used as the output | Passed |
| Both PNG sources reproduce their tracked RGB files exactly | Passed |
| Converted assets match the working prototype's raw images byte for byte | Passed |
| Full-screen and narrow-rectangle pixel order, masking, and row lengths | Passed |
| Source pixels remain unchanged after rendering | Passed |
| Null data, malformed lengths, invalid bounds, and uninitialized panel rejected | Passed |
| Command and pixel-transfer failures stop rendering | Passed |
| Slide count, invalid indices, and malformed embedded image lengths | Passed |
| Startup selects the business card; short releases advance and wrap | Passed |
| Press, untracked release, and long-release events do not navigate | Passed |
| Failed drawing preserves the selected index for the next navigation attempt | Passed |

Converter tests are included under `firmware/tools/tests` and run with `python -m unittest discover -s tools/tests -v` from `firmware`. Local RISC-V/QEMU harnesses exercise the actual driver, slide metadata, and application code with peripheral/event stubs. The application harness supplies button events directly; debounce timing is covered by the earlier input tests. These checks do not establish LCD output, physical QR readability, or real-time input handling during transfers.

### Hardware Acceptance

The application, bootloader, and updated partition table were flashed on COM4 on 2026-09-22. Written data hashes were verified and the board was reset. The revised QR background and translucent panel appearance were confirmed on the display. Remaining hardware checks:

- Confirm readable, correctly oriented business-card artwork at startup.
- Perform at least 20 short presses, checking exactly one change on each release and wraparound between both slides.
- Hold for more than 900 ms and confirm release logs without changing the slide.
- Scan the displayed QR and confirm the intended LinkedIn destination.
- Perform five full power cycles with LCD power on; confirm the business-card slide returns each time.
- Record draw duration and free/minimum/largest-block heap values after startup and repeated navigation.
- Enable startup diagnostics and finish the earlier edge-rendering and post-diagnostic button checks.

Synchronous drawing pauses button polling. Complete gestures during that interval may be missed. LCD recovery, incremental rendering, and input handling during animated transitions remain later-stage work.

### QR Background Update

The QR slide now composites the original black pattern over a 25-percent white panel on the business card's gold background. Local tests check every composited row against the expected gold and black colors, slide wraparound, malformed QR asset rejection, and failures on the first and middle rows. The source images remain unchanged. The revised appearance was confirmed on the display after reflashing. Physical QR scanning remains open.

The build-size measurements above include this QR update. Converter tests also verify that the original black-and-white panel retains a 24-pixel quiet border on all sides and reject multi-frame artwork.

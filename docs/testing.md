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

## LCD Recovery and Transitions : 2026-09-22

Status: complete; software and hardware checks passed.

### Build and Memory

The standard ESP-IDF v6.1 build produces a 1,111,632-byte application in the existing 2,097,152-byte partition, leaving 985,520 bytes free. The linker places the 4,800-byte tile-order array in DRAM. Drawing uses a 192-byte stack tile and the existing 960-byte driver buffer. Full QR redraws retain their separate 960-byte stack row. No full-screen framebuffer is allocated in the firmware.

Builds passed with startup diagnostics both disabled and enabled. The application logs initial drawing duration, transition duration, longest observed input-sampling interval, and heap readings. Timing and heap checks passed during hardware testing. This record contains pass/fail results rather than numerical readings. Panel reset, startup drawing, and recovery drawing remain synchronous.

### Integrated Software Checks

A local RISC-V/QEMU harness compiles the actual button, application controller, transitions, and slides modules. Synthetic raw button readings and timestamps pass through the real debounce logic. Stubbed LCD operations reconstruct output pixels and inject errors; they do not emulate electrical timing or the LCD's response to power loss. The harness remains local.

| Check | Result |
|---|---|
| Startup image and startup-held button release behavior | Passed |
| Invalid slide indices, region bounds, lengths, and null buffers | Passed |
| Malformed embedded artwork rejected before drawing | Passed |
| Twenty repeated transitions produce the expected final pixels | Passed |
| All 2,400 tiles drawn exactly once per completed reveal | Passed |
| QR tiles preserve the gold background, translucent panel, and black modules | Passed |
| One wipe strip or at most sixteen tiles per update | Passed |
| Early update does no work; delayed update does not catch up in a burst | Passed |
| Input wholly inside animation updates activity without queuing an action | Passed |
| A held press spanning completion is consumed until a stable release | Passed |
| A raw press first sampled after the final step is also suppressed | Passed |
| Long release reinitializes the panel and redraws either selected slide | Passed |
| Recovery preserves slide selection and does not recreate the SPI bus | Passed |
| Panel failure prevents redraw; a later recovery can succeed | Passed |
| Wipe/tile failure stops animation without advancing the selected slide | Passed |
| Recovery restores the selected image after a failed transition | Passed |
| Controller reinitialization cancels unfinished animation | Passed |

### Hardware Acceptance

This version was flashed on COM4 on 2026-09-22. Written data hashes were verified and the board was reset. Hardware checks were confirmed successful on the assembled device:

| Check | Result |
|---|---|
| LCD-only power-cycle recovery restores the selected slide | Passed |
| Repeated transitions complete without residual strips or tiles | Passed |
| Gestures during animation do not queue navigation or recovery | Passed |
| Holds spanning animation completion are consumed until release | Passed |
| A fresh gesture works after the release gate clears | Passed |
| QR scanning after transition and recovery | Passed |
| Transition timing, input-sampling gaps, and heap checks | Passed |

These results cover recovery and animation. The separate cold-start checks from earlier milestones remain open.

## Inactivity Sleep and Wake : 2026-09-22

Status: complete; software and hardware checks passed.

Software validation repeated on 2026-09-27: both ESP-IDF build configurations, power-manager and display-driver simulations, integrated navigation/recovery/sleep checks, malformed-asset rejection, and all seven image-converter tests passed.

### Build and Software Checks

The normal ESP-IDF v6.1 build produces a 1,121,456-byte application in the existing 2,097,152-byte partition, leaving 975,696 bytes free. Builds passed with startup diagnostics disabled and enabled. No full-screen framebuffer was added.

Local RISC-V/QEMU checks exercise the actual power manager, button, controller, transition, slide, and display-driver code. Sleep and peripherals are stubbed; these checks establish state handling and command sequencing, not real power consumption, electrical wake behavior, or LCD sleep timing.

| Check | Result |
|---|---|
| No entry at 299,999,999 us; entry begins at 300,000,000 us | Passed |
| Accepted input extends inactivity; held input and rendering defer entry | Passed |
| Stable raw release required before entry | Passed |
| Display-off precedes sleep-in; drawing blocked until panel reinitialization | Passed |
| 120 ms settling completes before processor sleep or restoration | Passed |
| Brief raw press or accepted activity during settling cancels entry | Passed |
| Fresh pin checks prevent entry with an already pressed button | Passed |
| Wake configuration failures and panel command errors propagate | Passed |
| Rejected sleep restores the panel and restarts inactivity | Passed |
| Ten simulated wake cycles preserve selection and expected pixels across both slides | Passed |
| Long-held wake gesture is consumed; a fresh gesture works after release | Passed |
| Failed restoration preserves selection and permits later manual recovery | Passed |
| Existing twenty-transition, QR composition, and recovery regressions | Passed |

### Hardware Acceptance

The sleep/wake firmware was flashed on COM4 on 2026-09-22. Written data hashes were verified and the board was reset. Hardware acceptance checks were confirmed complete on 2026-09-27. Functional sleep/wake checks are complete. Current consumption and physical wake latency are outside this release's measurement scope.

- [x] Leave each slide idle for the full 300 seconds and verify entry without a premature timeout.
- [x] Press before timeout and confirm a fresh five-minute interval after accepted activity.
- [x] Hold the button through timeout and confirm sleep is deferred.
- [x] Complete at least ten sleep/wake cycles across both slides with LCD power on.
- [x] Confirm a brief wake press restores the same slide without navigation.
- [x] Hold the wake button longer than 900 ms and confirm release does not trigger recovery or navigation.
- [x] Confirm a fresh short press and long-press recovery still work after waking.
- [x] Press during the panel-settling interval and verify restoration without an extra action.
- [x] Scan the QR after waking and check for residual or missing pixels.

The directly powered backlight remains on. Display sleep does not disconnect its supply, and software checks do not establish a numerical power reduction. Redraw logs measure panel initialization plus rendering after sleep returns; they are not a complete wake-latency measurement.

## Release Validation : 2026-09-27

All remaining functional hardware checks are confirmed complete, including reset and USB power-cycle behavior, five cold starts, display-edge rendering, and button operation after diagnostics. This supersedes earlier pending functional hardware entries. The clean-checkout build workflow is also confirmed complete. Timing and heap results are recorded below. Device media is included in the project and hardware documentation. Current consumption and physical wake latency are outside this release's measurement scope.

## Recorded Timing and Memory : 2026-09-27

Device: assembled XIAO ESP32-C3 card, USB connected to the host PC, ESP-IDF v6.1, normal display configuration with startup diagnostics disabled. Supply voltage was not measured. Five USB-triggered resets were followed by twenty manually triggered short-press transitions, allowing each animation to finish before the next press. These resets are not supply power cycles.

The firmware source corresponds to `fe8d997`. The existing flashed build reports `876373f-dirty` because it was compiled before that source was committed. Its logged ELF hash prefix matches the local ELF SHA-256 `7b347ccf140ae9c73c867ccd0ec7c1dc2d91cb9715d84b2629d5b310d8041b46`.

| Measurement | Samples | Result |
|---|---|---|
| Initial slide drawing | 5 resets | 238 ms on every run |
| Transition duration | 20 transitions | 2604.8 ms average; 2556–2676 ms range |
| Largest logged input-sampling gap during transitions | 20 transitions | 10004 us |
| Free heap after startup and completed transitions | 5 startup + 20 transition readings | 315,072 bytes at every reading |
| Minimum-ever free heap at those readings | Same samples | 315,072 bytes |
| Largest free internal allocation block | Same samples | 180,224 bytes |

Initial drawing time excludes earlier boot and panel initialization. Transition durations include the scheduled animation delays and drawing work. Sampling-gap results describe this run rather than a guaranteed worst-case bound. Unchanged heap readings show no observed growth in allocated heap during this test; they do not prove the absence of every memory defect. The captured logs contain no reported operation failures.

Whole-device current, supply voltage, and physical button-to-image wake latency were not measured by this serial capture. Raw captures and collection scripts are retained locally.

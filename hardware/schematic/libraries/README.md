# XIAO ESP32-C3 Libraries

The project includes local XIAO symbol and footprint libraries. Library tables use `${KIPRJMOD}` to resolve paths relative to the KiCad project.

## Sources

### Seeed Studio

- Download: https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/Seeeduino-XIAO-ESP32C3-KiCAD-Library.zip
- Retrieved: 2026-09-20.
- Files: `seeed/MOUDLE-SEEEDUINO-XIAO-ESP32C3.kicad_sym` and the footprint inside `seeed/xiao ESP32C3_PCB.pretty/`.
- The downloaded ZIP did not contain a license file. No new license is assigned to these third-party files by this repository.
- The symbol is unchanged. The footprint's unresolved external STEP-model reference was removed from the local copy; its geometry was retained.

### VectorSpaceHQ

- Source: https://github.com/VectorSpaceHQ/XIAO_ESP32C3
- Revision: `8e523d7cc48084e082f2eb0538eb0aa196e22073`.
- Retrieved: 2026-09-20.
- Files: `vectorspace/xiao_esp32c3.kicad_sym` and `vectorspace/XIAO_ESP32C3.pretty/xiao_esp32c3.kicad_mod`.
- Upstream `LICENSE` (GPL-3.0) and `README.md` are included in `vectorspace/`.
- The local upstream README links to the screenshot at the pinned revision.
- The symbol is unchanged. The footprint's absolute external STEP-model reference was removed from the local copy and the S-expression formatting normalized; its geometry was retained.

## Project-Specific Footprint

The PCB uses a modified version of the Seeed footprint with **14 through-hole pads**, whereas the downloaded Seeed original uses surface-mount pads. The two variants use different mounting styles.

`project/XIAO_Custom.pretty/MOUDLE14P-SMD-2.54-21X17.8MM.kicad_mod` was recovered from the embedded footprint in `Project_Buisiness_Card.kicad_pcb`. It retains the original item name for compatibility despite that name containing `SMD`. Board placement, net assignments, instance identifiers, and sheet metadata were removed; the reference placeholder was reset to `REF**`. Pad geometry and drawing data were retained.

The library nickname `xiao ESP32C3_PCB` now resolves to this project-specific library. This preserves the existing schematic assignment and board footprint identifier without changing the circuit or layout. The downloaded surface-mount version is registered separately as `Seeed_XIAO_Original` for reference.

The VectorSpaceHQ symbol's default footprint references `XIAO_ESP32C3:xiao_esp32c3`, which is also supplied. The existing placed schematic symbol overrides that default with the custom footprint. The through-hole assembly uses the project-specific assignment rather than the library symbol’s default footprint.

## Library Compatibility

- The downloaded VectorSpaceHQ symbol matches the embedded schematic symbol's 16 pin numbers, names, positions, and electrical types.
- The recovered custom footprint matches the board's 14 pad numbers, types, shapes, positions, sizes, drill sizes, and layers.
- Every URI in the project footprint and symbol library tables resolves to an existing local file or directory.
- External XIAO STEP models are not included; the board's standard KiCad model references still require KiCad's installed model libraries.

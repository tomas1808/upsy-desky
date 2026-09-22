# Firmware Configs

This directory contains [ESPHome](https://esphome.io) config files for the Upsy Desky.

It is organized like so:

- `base.yaml`: The base essential configuration, which contains components for reading the desk height, preset buttons, etc.
- `stock.yaml`: Inherits everything from `base.yaml` and adds components which are useful on stock firmware, such as the WiFi hotspot, web server, and Improv Serial
- `debug.yaml`: Inherits everything from `stock.yaml` and adds components which are useful for debugging
- `feiya.yaml`: Receive-only FEIYA height reader with encrypted Home Assistant API and Wi-Fi setup
- `feiya-control.yaml`: Adds panel controls and native preset recall/save buttons; requires the original panel to be physically disconnected
- `feiya-diagnostic.yaml`: USB-only, receive-only raw UART capture for investigating the FEIYA desk

## FEIYA FYK024-1S2

The measured pinout and protocol are recorded in the
[FEIYA bring-up guide](../docs/feiya-bringup.md). With the modified cable:

- RJ45 pin 1 / GPIO18 carries controller-to-panel segment display data.
- RJ45 pin 2 / GPIO17 carries panel-to-controller button frames, including a
  continuous idle stream at about 100 frames per second.
- Pin 3 is ground and pin 5 is the Upsy +5 V rail.

`feiya.yaml` reads height without driving the shared desk wiring. It publishes
height, display status and desk-data connectivity through the encrypted native
API. Blank standby frames preserve the last measured height. A bus timeout or
non-height display invalidates it. Height remains unknown after boot until the
panel first shows a numeric height. Formats above 100 cm still need a hardware
check; there are no remote movement controls in this configuration yet.

With the panel-side cable physically unplugged from Upsy, `feiya-control.yaml`
provides these controls:

Home Assistant's **Controls** section contains only the four preset recall
buttons. Both Up/Down pairs, Cancel Manual Move and all save buttons are in
**Configuration**. Restart remains diagnostic.
If an existing device still shows the previous grouping, reload its ESPHome
integration entry in Home Assistant, then refresh the device page.

- **Up / Down**: the existing 500 ms adjustment per tap, followed by release.
- **Up 5 s / Down 5 s**: a five-second adjustment per tap, followed by release.
- **Cancel Manual Move**: release an active short or long manual adjustment early.
  This does not cancel a controller-owned preset trip or interrupt a save.
- **Preset 1 / 2 / 3 / 4**: recall a stored position with a 200 ms key press.
  Key codes are `02`, `04`, `08`, `10`; full frames match the captured panel data.
- **Save Current Height to Preset 1 / 2 / 3 / 4** (configuration controls): hold M
  for 1 second, release for 300 ms, press the selected preset for 200 ms, release.

To set a preset, use Up/Down to position the desk, wait for movement to finish,
and press the corresponding Save Current Height button. The user confirmed
Save Current Height to Preset 2 worked on the physical desk. Slots 1, 3 and 4
use the same sequence with their respective captured key codes. The separate
M - Set button has been removed; saving handles M automatically.

Changing a preset stores the current physical position; no numeric height is
sent. No save or movement runs at boot or flashing. The new slot buttons and
five-second adjustments pass host tests but still need their own physical tests.

The sequencer rejects overlapping recall, save and manual requests. A save is
cancelled on stale controller data, Wi-Fi/API disconnect, shutdown or excessive
sequence delay; no delayed preset step is resumed later. M may temporarily
produce nonnumeric display frames, so these are accepted during the sequence.
Manual adjustments also release on invalid display data. Only 500 ms and
5000 ms durations are accepted; rejected/overlapping requests cannot change or
extend the duration already running. The controller determines native preset
travel and stores its memory. Height
sensors remain observational, with no automatic height-target movement.

The previous height-feedback controller overshot in the user's physical test
and has been removed. The cause of that overshoot is not established. Do not
restore it on the assumption that its host-side tests verified motor stopping.

Cancel Manual Move applies only to an active Up/Down adjustment. Releasing a
panel key is **not a verified cancel command for autonomous preset travel**. Wi-Fi/API disconnects and shutdown release
any current key press; they cannot be claimed to stop a controller-owned trip.
Test recalls beside the desk with clear travel before automating them. An
unwanted move may require switching off desk power until native cancellation
is identified. Reconnecting the panel requires receive-only firmware or physical
isolation of its command wire.

An automation can use Home Assistant's **Button: Press** action and select
any Preset 1–4 entity. No repeated button presses are required by this firmware.

Wi-Fi can be provisioned through the `Upsy Desky Setup` fallback access point.
Generate a private API key and AP/OTA passwords in git-ignored `secrets.yaml`
(see `secrets.yaml.example`). Do not commit that file. Once on the home network,
add the device using Home Assistant's ESPHome integration and its API key.

Build the FEIYA configs with **ESPHome 2025.12.6**. They include a compatibility
flag restoring the legacy API login reply needed by Home Assistant 2025.5.1
(`aioesphomeapi` 30.1.0). The connection still requires the encryption key;
there is no separate API password. Revisit this flag before upgrading ESPHome,
because it depends on that version's API implementation.

**Measured display frame:** `5A digit1 digit2 digit3 checksum`, where the checksum
is the sum of the three digit bytes modulo 256. Digits use standard active-high
seven-segment patterns, with bit 7 for the decimal point. Captured examples:
`5A 07 87 3F CD` = 77.0 cm; `5A 07 ED 7F 73` = 75.8 cm.

**Measured button frame:** `A5 00 buttons ~buttons FF`.
Idle/release uses `00`; up uses `20`; down uses `40`. The photographed preset
codes are in the bring-up guide; preset timing still needs live testing.

Decoder tests (no ESP32 required):

```sh
clang++ -std=c++17 -Wall -Wextra -Werror tests/feiya_protocol_test.cpp -o /tmp/feiya_protocol_test
/tmp/feiya_protocol_test
clang++ -std=c++17 -Wall -Wextra -Werror tests/feiya_preset_test.cpp -o /tmp/feiya_preset_test
/tmp/feiya_preset_test
```

## Addons

Major parts of the config are separated into "addons" so they can be easily included or excluded. The following addons are available:

- `presets.yaml`: Adds support for recalling and setting presets on the desk control box
- `runtime-config.yaml`: Adds support for runtime configuration options (you might want to remove this if you are configuring everything via ESPHome yaml)
- `bluetooth-proxy.yaml`: Contains the necessary configuration to use the Upsy Desky as a [Bluetooth Proxy](https://esphome.io/components/bluetooth_proxy.html)
- `stable-ids.yaml`: Contains configuration necessary to keep some entity IDs stable via the HTTP API

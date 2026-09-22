# FEIYA FYK024-1S2 bring-up

## Current status: four presets, short and long manual adjustments

The requested Home Assistant layout groups both Up/Down pairs, Cancel Manual
Move and all four Save Current Height buttons under Configuration. Only preset
recall buttons 1–4 remain ordinary controls; Restart remains diagnostic.
The category-only update compiled and was flashed with hash verification. A
read-only API check confirmed CONFIG for all nine setup buttons and NONE for
the four preset recalls; Restart remained DIAGNOSTIC. Entity names/IDs and
movement behavior are unchanged. The status helper now prints entity categories
to make this layout verifiable without triggering controls.
The user confirmed that reloading the ESPHome integration applied the new
grouping in Home Assistant. A device reset was not needed.

The user confirmed **Save Current Height to Preset 2 worked**. The M-hold,
release, preset sequence is therefore physically verified for that slot.
They requested all remaining presets, removal of M - Set, and five-second
Up/Down buttons alongside the existing short adjustments.

The new configuration includes recall/save buttons for all four slots. Slot 3
uses `A5 00 08 F7 FF`; slot 4 uses `A5 00 10 EF FF`, matching the photo. The
existing Up/Down controls retain their 500 ms duration and entity names. New
Up 5 s/Down 5 s controls use a bounded 5000 ms duration. Cancel Manual Move
releases an active manual jog early and does not interrupt saves or claim to
stop a controller-owned preset trip. The standalone M control is removed.

Tests cover all four exact recall/save frames, both manual durations, invalid
slots/durations, overlap rejection without changing the active duration, early
manual cancellation, save-sequence cancellation/delays and clock rollover.
The ESPHome 2025.12.6 build passed and was flashed over USB at 115200 baud with
hash verification. A read-only encrypted API check enumerated all 20 entities,
including all four recall/save pairs, the existing Up/Down buttons, Up 5 s,
Down 5 s and Cancel Manual Move. M - Set was absent. Desk data was connected,
online and in standby. No movement or memory-writing commands were sent during
verification. Physical tests of slots 3/4 and the new five-second moves remain
pending; save slot 2 was confirmed by the user before this update.


## Previous manual positioning and preset-save trial

The user now requests manual Up/Down controls to position the desk and native
preset saving. They tentatively recall holding M then pressing a preset and
explicitly asked to try it first rather than capture the original panel.

The trial uses M (`A5 00 01 FE FF`) for 1000 ms, idle/release for 300 ms, then
preset 1 or 2 for 200 ms, followed by idle/release. M is released before the
preset press; simultaneous M+preset behavior is not established. Save Current
Height to Preset 1/2 buttons invoke this sequence only when pressed by the user.
There is no confirmation parser for saved memory. If M fails to enter save
mode, the following slot command may recall an existing position instead.

Up/Down taps send their captured key codes for 500 ms then release. M - Set
sends M for 1000 ms as a separate panel button. Recall buttons 1/2 remain.
All these actions share one sequencer, rejecting overlaps, and cancel queued
steps on data loss or Wi-Fi/API disconnect. Save gaps remain busy, and a delayed
sequence is abandoned rather than emitting an orphaned slot command later.

Host tests cover exact M/slot bytes, hold/gap/press timings, manual jog expiry,
invalid commands, overlap rejection, cancellation, delays and clock rollover.
The ESPHome 2025.12.6 build passed and was flashed over USB at 115200 baud with
hash verification. A read-only encrypted API check enumerated all 14 entities,
including Up, Down, M - Set, Preset 1/2 and Save Current Height to Preset 1/2.
Desk data was connected and the display in standby. Physical save verification
is pending. Neither a save nor a manual movement has been triggered by the
assistant for this trial.


## Previous native-recall build

The user reported that the height-feedback controller went well past its target.
The cause was not established; its host-side tests did not verify physical
stopping. The existing Stop command was sent successfully before changing the
firmware, but physical stopping was not independently observed.

At the user's explicit request, the control firmware now implements only preset
1 and preset 2 recalls, from the photographed commands: `A5 00 02 FD FF` and
`A5 00 04 FB FF`. The user thinks these slots already contain 74/115 cm and asked
to implement them without further programming for now. Each recall is a 200 ms
button press followed by the measured idle/release stream. The press duration
is an initial choice, not a captured preset timing measurement.

The software target-height controller and its tests are removed. Numeric height
inputs, software movement status, and the misleading Stop control are removed
from the active configuration. Height reporting is observation only. No M/set,
up/down movement, or memory-writing command is exposed. Releasing a button is
not known to cancel a controller-owned preset trip. The first recall must be
observed beside the desk, with clear travel and access to its power switch.

Preset unit tests pass, covering exact captured bytes, allowed commands,
press timeout, release, rejected overlaps and clock rollover. The build passed
with ESPHome 2025.12.6 and was flashed at 115200 baud with hash verification.
A read-only encrypted API check using Home Assistant's matching client found
all nine entities: Preset 1 and Preset 2, diagnostic Restart, height, display
status/activity, desk data connectivity, online and IP address. No number inputs,
software motion status or Stop control were advertised. Desk data was connected
and the display in standby. Neither preset was triggered during verification;
physical recall behavior and the stored positions still need an attended test.

## Previous firmware history

Previous requested controls: Sitting Height input + Sitting button, Standing
Height input + Standing button, and Stop. Defaults are 74/115 cm, with saved
adjustments across reboots. Target Height/Go to Height and the fixed-height
buttons are removed from this configuration. Earlier prototype zero values
are migrated to valid defaults without triggering movement. This layout compiled
successfully and was flashed over USB at 115200 baud with hash verification.
A read-only encrypted API check enumerated Sitting, Standing, Stop and both
number inputs, reporting Sitting Height = 74.0 and Standing Height = 115.0.
Target Height, Go to Height and the fixed-height buttons were absent from the
device's entity list. No movement commands were sent during verification.

## Previous verification

Status: Home Assistant integration and initial jog firmware are verified. The
target-height controller passes host-side tests and its first Sit/Stand build
was compiled, flashed and verified through the encrypted API. The user chose
sitting at 74 cm and standing at 115 cm, while retaining Target Height, Go to
Height and Stop. The two fixed buttons compiled successfully and were flashed
with hash verification. A read-only API check enumerated all 13 entities,
including Sit (74 cm), Stand (115 cm), Go to Height, Stop and Target Height.
The saved custom Target Height value survived the update. The verification
sent no movement commands.
The requested fixed 70–80/110 cm buttons were cancelled before implementation.
The preceding input-box configuration
compiled successfully and was flashed over USB at 115200 baud with hash
verification. A read-only check using Home Assistant's matching API client
enumerated all 11 entities, including Target Height (initially 76 cm), Go to
Height and Stop. Desk data was connected and the display in standby. Physical
target movement verification remains pending. See the firmware README for
wake behavior, limits and controls.

Before installing the fixed 74/115 cm buttons, a read-only API check reported
115.0 cm, display in standby, desk data connected and motion status Stopped.
This is a live decoded height above 100 cm; the physical height and stopping
accuracy were not independently measured during that check.

For the prior jog build, the user confirmed the desk
entities appeared and both Up 0.5 s and Down 0.5 s moved the desk successfully.
A subsequent encrypted API check read 75.9 cm in standby, then received a live
update to 75.8 cm with the display active. The device and desk data were online.
The original panel remains disconnected; reconnecting it and testing the
handset's M/preset commands remain deferred.

Captured handset button traffic on pin 2 / GPIO17 and decoded controller
display traffic on pin 1 / GPIO18 at 9600 8N1. These measurements correct the
recalled pin labels. The original panel now works with receive-only firmware.
The user chose to leave the panel physically disconnected and use software
control for now; integrating it again is deferred. Supply voltage is unmeasured.
Receive-only diagnostic firmware was flashed and USB startup verified on
2026-09-22. The desk cables were disconnected by the user before flashing.
`firmware/feiya-diagnostic.yaml` passed configuration validation and a complete
ESP32 build with the locally installed ESPHome 2025.12.6. This verifies the
diagnostic firmware builds; it does not verify the cable or desk protocol.
The photographs identify a FEIYA FYK024-1S2 controller (Wuxi JDR), with a
29 V DC input and 24 V motor output. Those label voltages do **not** establish
the handset connector's supply or logic voltage.

## Evidence and remaining unknowns

The user has a multimeter and logic analyzer/oscilloscope and recalls this
mapping for the modified connectors:

| 8-pin contact | Recalled function | Upsy Desky connection |
| --- | --- | --- |
| 1 | Sleep (polarity and direction unknown) | GPIO18 |
| 2 | Recalled panel RX; capture proves button traffic is here | GPIO17 |
| 3 | GND | Ground |
| 4 | Recalled panel TX; no bytes in first capture | GPIO16 |
| 5 | +5 V | +5 V rail |
| 6–8 | No useful signals recalled | GPIO19, GPIO21, GPIO22 |

Second capture correction: pin 1 / GPIO18 carries repeated `5A 00 00 00 00`
frames at 9600 8N1 in standby, so the earlier sleep-line label is incorrect.
The second receive-only build observes this pin alongside GPIO17, without
driving either. It compiled, flashed with hash verification, and startup logs
confirmed both configured RX pins. A second movement capture matched the
`5A` frames with displayed heights (see below). Log file:
`firmware/.esphome/feiya-pin1-20260922-02.log` (git-ignored).

Observed/reported on 2026-09-22:

- Upsy is connected to the desk and this Mac via USB-C. With it in-line, the
  original panel has never worked correctly; up/down sometimes responded after
  different button sequences but currently fail, and pressing buttons flashes
  all display segments instead of showing height. The user suspects firmware
  left over from a failed configuration attempt.
- Replacing Upsy with a passive Ethernet coupler makes the desk work perfectly.
  This establishes that the adapted cable preserves the connections needed for
  normal panel operation. It does not by itself verify compatibility with Upsy's
  fixed power pins and pull-ups. Prioritize firmware interference and the board's
  electrical loading/power path over reconstructing all ten original contacts.
- Home Assistant at `http://192.168.1.63:8123/` returned HTTP 200 from this Mac.
  The user also supplied `http://homeassistant.local:8123`; hostname resolution
  and authenticated integration setup have not yet been checked.
- An eight-second USB serial observation returned a boot sequence and ESPHome
  2025.12.6, built January 17, 2026 at 05:26:56, project
  `tj_horner.upsy_desky` 4.0.2, with 4 MB flash. The log did not identify desk
  UART pins. The observation sent no serial payload and requested no reset;
  nevertheless boot output was observed, so do not assume serial-port opening
  is free of reset effects on this setup.
- The existing experimental YAML transmits on GPIO17 (reported pin 2) and
  receives on GPIO18 (reported sleep). This is a possible source of interference
  if installed, not proof of the current firmware's configuration or the fault.

Flashing outcome on 2026-09-22:

- The user confirmed both desk cables were disconnected and USB remained attached.
- A backup attempt at 460800 baud lost contact. A 115200-baud retry was cancelled
  at the user's request to stop bothering with the old image. No backup file was
  produced; do not claim rollback is available from an original flash backup.
- Wrote the built `firmware.factory.bin` at address `0x0` over USB at 115200 baud.
  Esptool reported `Hash of data verified` and exited successfully. Only the image
  region was erased/written; a whole-chip erase was not used.
- The subsequent 10-second USB capture confirmed successful startup and RX-only
  UARTs on GPIO17 and GPIO16, both 9600 8N1. No desk TX pin was configured.
  Logs: `firmware/.esphome/feiya-usb-startup-20260922.log` (git-ignored).
- Added `tools/capture-serial.py` for further timestamped USB captures; it was
  exercised successfully for the startup check. Its timestamps are host receive
  times for log lines, not precise desk UART byte timing.

Panel test after flashing: the user reports normal up/down operation and correct
height display with Upsy in-line after the requested desk-powered test. The user
explicitly skipped the pin 5 voltage measurement, so voltage remains unmeasured.
This strongly supports interference from the prior firmware as the cause of the
panel failure; it does not identify its exact installed pin configuration.

Next: reconnect USB and confirm the panel remains normal, then capture button
press/release and display traffic using the receive-only firmware. Establish
repeat/release behavior and return framing before implementing command injection
or a height decoder. If USB reconnection reintroduces the fault, investigate the
USB/desk power interaction and measure the supply before proceeding.

Use this as the working mapping instead of the earlier arbitrary GPIO17/18
capture pair. The first capture listened on GPIO17 and GPIO16, leaving GPIO18
undriven. Although the user recalled RX/TX from the panel perspective, the
captured button presses are on GPIO17, not GPIO16. Use this measured mapping
for further work. Do not infer sleep polarity or drive that line from its name.

First confirm whether the panel has worked through the modified cables and
Upsy Desky. With Upsy and USB disconnected, verify the recalled ground with
power off; then select DC volts and measure pin 5 relative to pin 3 with desk
power on. Also record pins 1, 2 and 4 at idle and on panel wake-up. Only attach
an analyzer after checking its input voltage rating against the measured levels.
The full continuity worksheet below is available if these checks disagree with
the recalled mapping; a complete 10-pin reconstruction is not a prerequisite
if the needed connections and working panel pass-through can be established.

The handwritten capture records handset-to-controller messages:

| Button | Hex bytes |
| --- | --- |
| M / Set | `A5 00 01 FE FF` |
| Preset 1 | `A5 00 02 FD FF` |
| Preset 2 | `A5 00 04 FB FF` |
| Preset 3 | `A5 00 08 F7 FF` |
| Preset 4 | `A5 00 10 EF FF` |
| Up | `A5 00 20 DF FF` |
| Down | `A5 00 40 BF FF` |

For these captured messages, byte 3 is the bitwise complement of byte 2.
This does not establish a general checksum algorithm for other packet types.
The original notes alone did not give baud rate, electrical polarity, repeat
interval, release message, stop behavior, or which wire carries the messages.
The first live capture established the following additional facts.

### First live button capture (2026-09-22)

Log: `firmware/.esphome/feiya-buttons-20260922-01.log` (git-ignored).

- GPIO17 (RJ45 pin 2) carries valid five-byte handset frames at 9600 8N1.
- Idle and button release: `A5 00 00 FF FF`, repeated continuously, including
  while the panel display is blank. Approximately 30 frames per 0.30 seconds
  gives about 100 frames/second. This estimate uses host log chunk timestamps;
  individual bit timing still requires a logic analyzer.
- UP: 146 captured `A5 00 20 DF FF` frames, then idle frames. The user reported
  a displayed height of 76.9 cm after release.
- DOWN: 182 captured `A5 00 40 BF FF` frames, then idle frames. The user reported
  a displayed height of 75.9 cm after release.
- The user confirmed motion stopped upon releasing both buttons. The starting
  height was unavailable because the standby display was blank.
- GPIO16 (RJ45 pin 4) produced no captured bytes, even while the display was
  active and the desk moved. Do not assign a display protocol to this line yet.
- No preset or M button was pressed in this capture.

The continuous idle command stream matters for remote control: a second UART
transmitter cannot simply be enabled on the shared command wire. It risks
electrical contention and interleaves remote commands with the handset's idle
frames. A separately wired handset-input/controller-output serial proxy may be
needed; do not enable the experimental direct-transmit preset buttons.

### Display capture and decoder

The user reported 77.0 cm after UP and 75.8 cm after DOWN in the second capture.
These match `5A 07 87 3F CD` and `5A 07 ED 7F 73`, respectively. The frame format
is `5A digit1 digit2 digit3 checksum`, with standard active-high seven-segment
patterns, bit 7 for each digit's decimal point, and checksum equal to the sum of
the three raw digit bytes modulo 256. No `54` header was observed.

The new `firmware/components/feiya_desk` decoder validates checksums, recovers
after corrupt/truncated frames and expires partial packets after a gap. It treats
all-zero digits as standby, retaining the last known height only during a live
connection. Non-height displays and loss of valid frames invalidate the height.
Power-on height is unknown until a numeric display frame arrives; it is not
restored from flash. Accepted numeric heights are bounded to 20–200 cm to reject
obviously unrelated numeric displays. Heights above 100 cm are supported by the
generic digit decoder but still need a physical desk check.

Validation: host tests exercise both captured endpoint heights, blank/error
patterns, checksums, resynchronization, timeouts and clock wraparound. Replaying
the full captured display stream produced 35,657 valid frames: 2,423 numeric and
33,234 blank, a 75.8–77.0 cm range and a final numeric height of 75.8 cm.

### Software-only control chosen by user

The user reported pin 6 is connected to something unknown and chose to defer
cable changes. They subsequently confirmed the panel-side cable was unplugged,
leaving the controller cable and USB connected. `feiya-control.yaml` is built for
this exact arrangement: RX GPIO18, TX GPIO17. Do not reconnect the panel while
this transmit-enabled firmware is installed. Pin 6 is not used or driven.

The first controls are Up 0.5 s, Down 0.5 s and Stop. The firmware transmits the
observed idle/release command at startup and between jogs, on an approximately
10 ms schedule. A jog requires recent valid display/standby traffic, expires
after a nominal 500 ms and is cancelled on invalid/stale display data or Wi-Fi/API
disconnect. Repeated/overlapping requests cannot extend a running jog. The user
confirmed both jog buttons work on the physical desk. The separate Stop button
and disconnect/stale-data cancellation have not been tested during actual motion;
the command deadline does not measure motor deceleration. Presets and M/reset
are not exposed.

The encrypted API, setup AP and OTA credentials are generated uniquely in
git-ignored `firmware/secrets.yaml`. The fallback AP is named `Upsy Desky Setup`.

The control firmware was compiled and flashed with hash verification on
2026-09-22 after the user confirmed physical panel disconnection. It automatically
joined the saved home Wi-Fi; consequently no setup AP appeared or was needed.
Device address: `192.168.1.53`, hostname `upsy-desky-feiya.local`, MAC
`F0:24:F9:E1:64:A4`. A direct encrypted ESPHome API check succeeded and listed the
Up 0.5 s, Down 0.5 s and Stop buttons plus height/status entities. Initial states:
desk data connected = true, display active = false, display status = Standby,
height = unknown, as expected until the first numeric frame after boot.

Home Assistant at `192.168.1.63:8123` attempted a connection with a failing
handshake. The user was given the new API encryption key and asked to add or
reconfigure the ESPHome integration for `192.168.1.53:6053`. Do not confuse the
setup AP password with the API encryption key. Integration and movement were
subsequently confirmed after the compatibility fix below. USB startup/network logs are saved in
`firmware/.esphome/feiya-network-20260922-03.log` (git-ignored).

### Home Assistant 2025.5.1 API compatibility

After adding the device with the correct encryption key, the user saw no
ESPHome devices or entities. A separate router presence entity reported
"Detected"; that does not establish an ESPHome API connection. USB logs identify
the Home Assistant client as 2025.5.1. Its
[official manifest](https://github.com/home-assistant/core/blob/2025.5.1/homeassistant/components/esphome/manifest.json)
pins `aioesphomeapi==30.1.0`.

A read-only connection test with that exact client reproduced the failure:
login waited for the legacy `ConnectResponse` until the test's 15-second timeout.
The current ESPHome Python client connected and enumerated all ten entities.
Inspection of both client versions and the ESPHome 2025.12.6 API source shows
that the older client requires this reply, while the newer one does not wait
for it. Firmware omits the handler unless `USE_API_PASSWORD` is defined.

`feiya.yaml` now sets that build flag to retain the legacy handler with its
default empty password. Noise encryption still requires the existing API key;
no separate API password is configured. This workaround is specific to the
tested ESPHome 2025.12.6 API implementation. Reassess it before changing compiler
versions.

The compatibility build compiled successfully and was flashed over USB at
115200 baud with hash verification. Both `aioesphomeapi` 30.1.0 and the current
ESPHome Python client then connected with the existing encryption key,
enumerated all ten entities and received live states: desk data connected,
online, display in standby, height unknown. This resolves the reproduced legacy
client login failure. The user subsequently confirmed that the actual Home
Assistant entry exposes the entities and reported that both Up 0.5 s and Down
0.5 s worked on the desk. A follow-up encrypted API read returned height 75.9 cm,
online = true, desk data connected = true and display status = Standby. During
the same observation it received a live update to 75.8 cm with the display
active. No movement commands were sent by this verification script.

The user recalls segment-encoded display traffic. The existing experimental
`firmware/addons/lcd-segment-desk.yaml` assumes a five-byte `54` header packet,
standard segment bit order and a decimal-point bit, and ignores its presumed
checksum. None of those details are established by the supplied captures.
Its parent config and comments also disagree about the TX/RX pins. Do not use
that experimental config as evidence of the cable mapping or a verified height
decoder. Capture return traffic with the displayed height before implementing
height reporting or movement to a target height.

## Fixed Upsy Desky wiring

Both RJ45 sockets connect straight through, contact for contact. The ESP32 taps
six signal lines through MOSFET level shifters with pull-ups to the board rails.
This is not a pair of independent serial ports separated by a software bridge.
Traffic to the panel, including display data, passes through without decoding.

| RJ45 contact | Board connection |
| --- | --- |
| 1 | GPIO18 through level shifter |
| 2 | GPIO17 through level shifter |
| 3 | Ground, fixed |
| 4 | GPIO16 through level shifter |
| 5 | +5 V rail, fixed |
| 6 | GPIO19 through level shifter |
| 7 | GPIO21 through level shifter |
| 8 | GPIO22 through level shifter |

Sources: `pcb/upsy-desky.kicad_sch`, `pcb/schematic.jpg`, and the manufacturer's
[GPIO table](https://upsy-desky.tjhorner.dev/docs/reference/gpio/).
Software cannot relocate ground or power. The board's pull-ups exist even in
receive-only firmware. Verify electrical compatibility before attaching it.

## Recover the modified cable mapping

1. Unplug desk power and USB. Disconnect both cable halves from the controller,
   panel, and Upsy Desky before using resistance/continuity mode.
2. Establish an unambiguous contact-number convention with photographs of the
   connectors. For an 8P8C plug, look down onto the gold-contact side with the
   latch underneath and the cable pointing toward you: contacts 1–8 run left
   to right. Photograph the 10-pin connector before assigning its numbers;
   do not assume it is the same style or orientation.
3. Measure each 8-pin contact to each original 10-pin contact, on **both** cable
   halves. Record open contacts and any contacts joined together. Wire colors
   alone are not proof of continuity.

| 8-pin contact | Controller half: original contact | Panel half: original contact |
| --- | --- | --- |
| 1 | ? | ? |
| 2 | ? | ? |
| 3 (GND on Upsy) | ? | ? |
| 4 | ? | ? |
| 5 (+5 V on Upsy) | ? | ? |
| 6 | ? | ? |
| 7 | ? | ? |
| 8 | ? | ? |

Record what happened to the other two original conductors as well. An omitted
conductor may still be necessary for the original panel.

The next measurement is DC voltage on the **low-voltage handset connector**,
with Upsy Desky and USB disconnected, relative to an identified signal ground.
Select DC volts before applying power. Establish ground from cable/board
evidence first; do not guess it from a wire color or a low voltage reading.
Record the supply, each signal's idle voltage, and behavior on a panel button
press. Do not open the motor controller or probe its mains supply. Use insulated
breakouts if the connector contacts cannot be probed without shorting neighbors.

## Receive-only capture

`firmware/feiya-diagnostic.yaml` configures two input-only UARTs and USB logging.
It deliberately has no desk TX, movement entities, startup commands or height
decoder. It currently listens on GPIO17/GPIO18 (pins 2 and 1), with readable
`A5` and `5A` streams at 9600 8N1. The initial GPIO17/GPIO16 capture found button
commands on GPIO17 and silence on GPIO16. Two UARTs plus the USB logger use the
original ESP32's three UARTs.
After validating cable voltages and mapping, change the substitutions to the
two measured signal pins. Other candidate GPIOs are in the table above.

From the repository root:

```sh
esphome config firmware/feiya-diagnostic.yaml
esphome compile firmware/feiya-diagnostic.yaml
```

Only after the cable checks, with Upsy disconnected from the desk while flashing:

```sh
esphome upload firmware/feiya-diagnostic.yaml --device /dev/cu.usbserial-0001
esphome logs firmware/feiya-diagnostic.yaml --device /dev/cu.usbserial-0001
```

The serial device path is the one observed on this Mac; confirm its identity
before upload. Reconnect the verified desk cables with power off before testing.
Log labels A/B identify GPIOs, not assumed traffic directions. The line carrying
the recorded `A5` commands identifies handset-to-controller traffic. Chunk
boundaries caused by timeouts or the 150-byte limit are not packet boundaries.
The config uses ESPHome's documented
[receive-only UART debugging](https://esphome.io/components/uart/#debugging).

Capture idle, a brief up press and release, a brief down press and release, and
one preset recall while attended with clear travel. Note timestamps, displayed
heights, when each button is released and when motion stops. Include at least
three different displayed heights, and a height above 100 cm if safely reachable,
to resolve digit count and decimal placement. Do not run a reset/calibration or
hold M while collecting ordinary movement samples. Observe the controller's
labelled duty cycle of 2 minutes on / 18 minutes off.

## Enabling control and Home Assistant

Before driving a line shared with the handset, determine whether it is an
open-drain bus or a push-pull UART. A permanently enabled ESP TX can contend
with the handset output. Depending on those measurements, control may need a
tri-stated output with appropriate arbitration or physical isolation/re-routing
of the command wire. GPIO remapping alone cannot split the PCB's shared net.
Confirm press/repeat/release and stop behavior before offering motion controls.

Once that is verified, the operational ESPHome config can expose the four desk
presets and any verified movement/height features through its native API. Use
local `firmware/secrets.yaml` (already git-ignored) for Wi-Fi credentials, a
generated API encryption key and an OTA password; do not reuse the fixed sample
key in the experimental config. No Home Assistant password belongs in firmware.

In Home Assistant, use Settings → Devices & services → Add integration → ESPHome
if discovery does not offer the device. Supply the device's IP address, default
API port 6053 and its encryption key. Verify availability and readings, then
test one attended preset before creating automations. The USB diagnostic config
does not yet provide a network API or Home Assistant controls. See the official
[ESPHome integration documentation](https://www.home-assistant.io/integrations/esphome/).

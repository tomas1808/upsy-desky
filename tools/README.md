# Tools

## `feiya-api-status.py`

Lists the FEIYA firmware's entities and observes their states through the native
encrypted ESPHome API. It reads `feiya_api_key` from the local git-ignored secrets
file and sends no movement commands. Requires `aioesphomeapi` and PyYAML, available
in the ESPHome Python environment. Example:

```sh
python feiya-api-status.py --host 192.168.1.53 --duration 10
```

## `capture-serial.py`

Records ESPHome USB serial logs to a new, timestamped text file for desk protocol
analysis. Requires Python and `pyserial` (available in the ESPHome environment).
For example, with the receive-only diagnostic firmware installed:

```sh
python capture-serial.py --port /dev/cu.usbserial-0001 --duration 60 --output ../firmware/.esphome/desk-capture.log
```

The output file must not already exist. Host timestamps indicate when log lines
were received, not individual UART bit/byte timing. The script sends no serial
data and requests no reset, but opening a port may still toggle hardware control
lines on some USB drivers. Firmware logs may contain network details; keep
captures in the git-ignored `.esphome` directory.

## `oem-flash.sh`

This script is used before shipment to flash the Upsy Desky with the stock firmware, and to reprogram the CP2102 USB-to-serial chip with the correct name and serial (based on MAC address).

## `test-flash.sh`

Flashes the test firmware with PlatformIO (see `../test-firmware`).

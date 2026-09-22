#!/usr/bin/env python3
"""Record ESPHome USB logs without sending serial data (requires pyserial)."""

import argparse
from datetime import datetime, timezone
import os
from pathlib import Path
import time

import serial


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/cu.usbserial-0001")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--duration", type=float, default=30)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    if not 0 < args.duration <= 3600:
        parser.error("--duration must be greater than zero and at most 3600 seconds")

    # Never overwrite a previous capture; it may be needed for protocol analysis.
    fd = os.open(args.output, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    with os.fdopen(fd, "w", encoding="utf-8", buffering=1) as log:
        port = serial.Serial(port=None, baudrate=args.baud, timeout=0.2)
        # Do not request a reset. Opening the port can still glitch the hardware
        # control lines on some USB drivers; use with the receive-only firmware.
        port.dtr = False
        port.rts = False
        port.port = args.port
        total = 0
        pending = bytearray()

        def record(data):
            timestamp = datetime.now(timezone.utc).isoformat(timespec="milliseconds")
            log.write(f"{timestamp} {data.decode('utf-8', errors='replace').rstrip(chr(13))}\n")

        try:
            port.open()
            print(f"Recording {args.port} for {args.duration:g}s to {args.output}", flush=True)
            deadline = time.monotonic() + args.duration
            while time.monotonic() < deadline:
                chunk = port.read(min(max(port.in_waiting, 1), 4096))
                total += len(chunk)
                pending.extend(chunk)
                while b"\n" in pending:
                    line, _, rest = pending.partition(b"\n")
                    record(line)
                    pending = bytearray(rest)
                if len(pending) >= 8192:
                    record(pending)
                    pending.clear()
        except KeyboardInterrupt:
            print("Capture interrupted; preserving received logs.", flush=True)
        finally:
            port.close()
            if pending:
                record(pending)
        print(f"Saved {total} serial bytes to {args.output}", flush=True)


if __name__ == "__main__":
    main()

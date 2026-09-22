#!/usr/bin/env python3
"""Read FEIYA ESPHome entities/states; never sends movement commands."""

import argparse
import asyncio
from pathlib import Path

import aioesphomeapi
import yaml


async def observe(args):
    credentials = yaml.safe_load(args.secrets.read_text())
    client = aioesphomeapi.APIClient(
        args.host, 6053, None, noise_psk=credentials["feiya_api_key"],
        expected_name="upsy-desky-feiya", client_info="FEIYA read-only verification"
    )
    try:
        await asyncio.wait_for(client.connect(login=True), timeout=15)
        info = await client.device_info()
        print(f"Connected: {info.name}, MAC {info.mac_address}", flush=True)
        entities, _ = await client.list_entities_services()
        names = {entity.key: entity.name for entity in entities}
        for entity in entities:
            category = getattr(getattr(entity, "entity_category", None), "name", "NONE")
            print(f"{type(entity).__name__}: {entity.name} ({entity.object_id}), category={category}", flush=True)

        def on_state(state):
            value = getattr(state, "state", state)
            print(f"State: {names.get(state.key, state.key)} = {value}", flush=True)

        client.subscribe_states(on_state)
        await asyncio.sleep(args.duration)
    finally:
        await client.disconnect()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="upsy-desky-feiya.local")
    parser.add_argument("--secrets", type=Path, default=Path(__file__).resolve().parents[1] / "firmware/secrets.yaml")
    parser.add_argument("--duration", type=float, default=5)
    args = parser.parse_args()
    if not 0 < args.duration <= 600:
        parser.error("--duration must be greater than zero and at most 600 seconds")
    asyncio.run(observe(args))


if __name__ == "__main__":
    main()

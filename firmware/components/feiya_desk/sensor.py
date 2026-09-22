import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor, text_sensor, uart
from esphome.const import DEVICE_CLASS_DISTANCE, STATE_CLASS_MEASUREMENT
from . import FeiyaDisplay

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "text_sensor"]

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        FeiyaDisplay,
        unit_of_measurement="cm",
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_DISTANCE,
        state_class=STATE_CLASS_MEASUREMENT,
        icon="mdi:human-male-height",
    )
    .extend(
        {
            cv.Optional("display_active"): binary_sensor.binary_sensor_schema(),
            cv.Optional("bus_connected"): binary_sensor.binary_sensor_schema(
                device_class="connectivity", entity_category="diagnostic"
            ),
            cv.Optional("display_status"): text_sensor.text_sensor_schema(
                entity_category="diagnostic"
            ),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "feiya_desk", baud_rate=9600, require_rx=True, data_bits=8, parity=None, stop_bits=1
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    uart.request_wake_loop_on_rx()
    for key in ("display_active", "bus_connected"):
        if key in config:
            entity = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(var, f"set_{key}")(entity))
    if "display_status" in config:
        entity = await text_sensor.new_text_sensor(config["display_status"])
        cg.add(var.set_display_status(entity))

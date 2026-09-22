"""Support for the captured FEIYA FYK024-1S2 display protocol."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
ns = cg.esphome_ns.namespace("feiya_desk")
FeiyaDisplay = ns.class_("FeiyaDisplay", sensor.Sensor, cg.Component, uart.UARTDevice)
FeiyaControl = ns.class_("FeiyaControl", cg.Component, uart.UARTDevice)

# This explicit setting documents a physical prerequisite; it cannot detect
# whether someone reconnects the panel to the shared command wire.
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(FeiyaControl),
            cv.Required("display_id"): cv.use_id(FeiyaDisplay),
            cv.Required("panel_disconnected"): cv.All(cv.boolean, cv.one_of(True)),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "feiya_desk control", baud_rate=9600, require_tx=True, data_bits=8, parity=None, stop_bits=1
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    display = await cg.get_variable(config["display_id"])
    cg.add(var.set_display(display))

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

techfine_ns = cg.esphome_ns.namespace("techfine_uart")
TechfineUART = techfine_ns.class_("TechfineUART", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(TechfineUART),
    cv.Required("uart_id"): cv.use_id(uart.UARTComponent),

    cv.Required("pv_voltage"): cv.use_id(sensor.Sensor),
    cv.Required("batt_voltage"): cv.use_id(sensor.Sensor),
    cv.Required("charge_current"): cv.use_id(sensor.Sensor),
    cv.Required("mcu_temp"): cv.use_id(sensor.Sensor),
    cv.Required("heatsink_temp"): cv.use_id(sensor.Sensor),
    cv.Required("daily_kwh"): cv.use_id(sensor.Sensor),
    cv.Required("total_kwh"): cv.use_id(sensor.Sensor),
    cv.Required("pv_power"): cv.use_id(sensor.Sensor),

    cv.Optional("update_interval", default="3s"): cv.positive_time_period_milliseconds,
})

async def to_code(config):
    # UART device / component setup
    uart_comp = await cg.get_variable(config["uart_id"])
    var = cg.new_Pvariable(config[CONF_ID], uart_comp)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # Resolve all sensor IDs into actual C++ sensor pointers
    pv_voltage = await cg.get_variable(config["pv_voltage"])
    batt_voltage = await cg.get_variable(config["batt_voltage"])
    charge_current = await cg.get_variable(config["charge_current"])
    mcu_temp = await cg.get_variable(config["mcu_temp"])
    heatsink_temp = await cg.get_variable(config["heatsink_temp"])
    daily_kwh = await cg.get_variable(config["daily_kwh"])
    total_kwh = await cg.get_variable(config["total_kwh"])
    pv_power = await cg.get_variable(config["pv_power"])

    # Pass pointers to C++
    cg.add(var.set_sensors(
        pv_voltage,
        batt_voltage,
        charge_current,
        mcu_temp,
        heatsink_temp,
        daily_kwh,
        total_kwh,
        pv_power
    ))

import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import *
from esphome.components import climate
from esphome.components import sensor
from esphome.components import binary_sensor
from esphome import pins

openthermgw_ns = cg.esphome_ns.namespace("opentherm")
OpenThermGWComponent = openthermgw_ns.class_("OpenThermGWClimate", cg.Component)

AUTO_LOAD = ["sensor", "climate", "binary_sensor"]
CONF_HUB_ID = "opentherm"

UNIT_HOURS = "h"
UNIT_BAR = "bar"

CONF_IS_CH2_ACTIVE = "is_ch2_active"
CONF_IS_CH_ACTIVE = "is_ch_active"
CONF_IS_COOLING_ACTIVE = "is_cooling_active"
CONF_IS_DHW_ACTIVE = "is_dhw_active"
CONF_IS_DIAGNOSTIC_EVENT = "is_diagnostic_event"
CONF_IS_FAULT_INDICATION = "is_fault_indication"
CONF_IS_FLAME_ON = "is_flame_on"
CONF_BOILER_WATER_TEMP = "boiler_water_temp"
CONF_BURNER_OPERATION_HOURS = "burner_operation_hours"
CONF_BURNER_STARTS = "burner_starts"
CONF_CH_PUMP_OPERATION_HOURS = "ch_pump_operation_hours"
CONF_CH_PUMP_STARTS = "ch_pump_starts"
CONF_CH_WATER_PRESSURE = "ch_water_pressure"
CONF_DHW2_TEMPERATURE = "dhw2_temperature"
CONF_DHW_BURNER_OPERATION_HOURS = "dhw_burner_operation_hours"
CONF_DHW_BURNER_STARTS = "dhw_burner_starts"
CONF_DHW_FLOW_RATE = "dhw_flow_rate"
CONF_DHW_PUMP_VALVE_OPERATION_HOURS = "dhw_pump_valve_operation_hours"
CONF_DHW_PUMP_VALVE_STARTS = "dhw_pump_valve_starts"
CONF_DHW_TEMPERATURE = "dhw_temperature"
CONF_EXHAUST_TEMPERATURE = "exhaust_temperature"
CONF_FLOW_TEMPERATURE_CH2 = "flow_temperature_ch2"
CONF_OUTSIDE_AIR_TEMPERATURE = "outside_air_temperature"
CONF_RELATIVE_MODULATION_LEVEL = "relative_modulation_level"
CONF_RETURN_WATER_TEMPERATURE = "return_water_temperature"
CONF_SOLAR_COLLECTOR_TEMPERATURE = "solar_collector_temperature"
CONF_SOLAR_STORAGE_TEMPERATURE = "solar_storage_temperature"
CONF_THERMOSTAT_IN_PIN = "thermostat_in_pin"
CONF_THERMOSTAT_OUT_PIN = "thermostat_out_pin"
CONF_BOILER_IN_PIN = "boiler_in_pin"
CONF_BOILER_OUT_PIN = "boiler_out_pin"

helper_opentherm_list = [
    CONF_BOILER_WATER_TEMP,
    CONF_BURNER_OPERATION_HOURS,
    CONF_BURNER_STARTS,
    CONF_CH_PUMP_OPERATION_HOURS,
    CONF_CH_PUMP_STARTS,
    CONF_CH_WATER_PRESSURE,
    CONF_DHW2_TEMPERATURE,
    CONF_DHW_BURNER_OPERATION_HOURS,
    CONF_DHW_BURNER_STARTS,
    CONF_DHW_FLOW_RATE,
    CONF_DHW_PUMP_VALVE_OPERATION_HOURS,
    CONF_DHW_PUMP_VALVE_STARTS,
    CONF_DHW_TEMPERATURE,
    CONF_EXHAUST_TEMPERATURE,
    CONF_FLOW_TEMPERATURE_CH2,
    CONF_IS_CH2_ACTIVE,
    CONF_IS_CH_ACTIVE,
    CONF_IS_COOLING_ACTIVE,
    CONF_IS_DHW_ACTIVE,
    CONF_IS_DIAGNOSTIC_EVENT,
    CONF_IS_FAULT_INDICATION,
    CONF_IS_FLAME_ON,
    CONF_OUTSIDE_AIR_TEMPERATURE,
    CONF_RELATIVE_MODULATION_LEVEL,
    CONF_RETURN_WATER_TEMPERATURE,
    CONF_SOLAR_COLLECTOR_TEMPERATURE,
    CONF_SOLAR_STORAGE_TEMPERATURE,
]

opentherm_sensors_schemas = cv.Schema(
    {
        cv.Optional(CONF_BOILER_WATER_TEMP): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_BURNER_OPERATION_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOURS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ).extend(),
        cv.Optional(CONF_BURNER_STARTS): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_CH_PUMP_OPERATION_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOURS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ).extend(),
        cv.Optional(CONF_CH_PUMP_STARTS): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_CH_WATER_PRESSURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_PRESSURE,
            unit_of_measurement=UNIT_BAR,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_DHW2_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_DHW_BURNER_OPERATION_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOURS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ).extend(),
        cv.Optional(CONF_DHW_BURNER_STARTS): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_DHW_FLOW_RATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_DHW_PUMP_VALVE_OPERATION_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOURS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ).extend(),
        cv.Optional(CONF_DHW_PUMP_VALVE_STARTS): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_DHW_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_EXHAUST_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_FLOW_TEMPERATURE_CH2): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_OUTSIDE_AIR_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_RELATIVE_MODULATION_LEVEL): sensor.sensor_schema(
            icon=ICON_PERCENT,
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_RETURN_WATER_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_SOLAR_COLLECTOR_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_SOLAR_STORAGE_TEMPERATURE): sensor.sensor_schema(
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ).extend(),
        cv.Optional(CONF_IS_CH2_ACTIVE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_CH_ACTIVE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_COOLING_ACTIVE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_DHW_ACTIVE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_DIAGNOSTIC_EVENT): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_FAULT_INDICATION): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
        cv.Optional(CONF_IS_FLAME_ON): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_EMPTY
        ).extend(),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(OpenThermGWComponent),
            cv.Required(CONF_THERMOSTAT_IN_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_THERMOSTAT_OUT_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_BOILER_IN_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_BOILER_OUT_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(opentherm_sensors_schemas)
    .extend(cv.COMPONENT_SCHEMA)
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    thermostat_in_pin = yield cg.gpio_pin_expression(config[CONF_THERMOSTAT_IN_PIN])
    cg.add(var.set_thermostat_in_pin(thermostat_in_pin))
    thermostat_out_pin = yield cg.gpio_pin_expression(config[CONF_THERMOSTAT_OUT_PIN])
    cg.add(var.set_thermostat_out_pin(thermostat_out_pin))
    boiler_in_pin = yield cg.gpio_pin_expression(config[CONF_BOILER_IN_PIN])
    cg.add(var.set_boiler_in_pin(boiler_in_pin))
    boiler_out_pin = yield cg.gpio_pin_expression(config[CONF_BOILER_OUT_PIN])
    cg.add(var.set_boiler_out_pin(boiler_out_pin))
    for k in helper_opentherm_list:
        if k in config:
            sens = None
            if "is_" in k:
                sens = yield binary_sensor.new_binary_sensor(config[k])
            else:
                sens = yield sensor.new_sensor(config[k])
            func = getattr(var, "set_" + k)
            cg.add(func(sens))

    cg.add(cg.App.register_climate(var))

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import TimePeriod
from esphome.components import binary_sensor, sensor , light
from esphome.const import (
    CONF_SENSOR_ID,
    CONF_THRESHOLD,
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
)

ext_threshold_ns = cg.esphome_ns.namespace("ext_threshold")

ExtThresholdBinarySensor = ext_threshold_ns.class_(
    "ExtThresholdBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONF_ANALOG = "analog"          #original analog_sensor parameters
CONF_ANALOG_FEEDBACK = "feedback"
CONF_TIME   = "time"            #pulse time output
CONF_AMPLITUDE  = "amplitude"   #pulse amplitude output
CONF_COUNT  = "count"           #pulse count output
CONF_ABSOLUTE ="absolute"
CONF_CALIBRATION ="calibration"
CONF_CALIBRATION_TIME ="calibration_time"
CONF_FEEDBACK_LIGHT_ID ="feedback_light_id"

CONF_UPPER = "upper"
CONF_LOWER = "lower"
CONF_MIN_UP_TIME = "on_min"
CONF_MAX_UP_TIME = "on_max"
CONF_LOW_OFF_TIME = "off_min"

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ExtThresholdBinarySensor),
        cv.Optional(CONF_ABSOLUTE, default=False): cv.boolean,
        cv.Required(CONF_THRESHOLD): cv.Any(
            cv.float_,
            cv.Schema(
                {cv.Required(CONF_UPPER): cv.float_, cv.Required(CONF_LOWER): cv.float_}
            ),
        ),
        cv.Required(CONF_ANALOG): sensor.sensor_schema(
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                {
                    cv.Required(CONF_SENSOR_ID): cv.use_id(sensor.Sensor),
                    cv.Optional(CONF_ANALOG_FEEDBACK):  sensor.sensor_schema(),
                }
            ),
        cv.Optional(CONF_FEEDBACK_LIGHT_ID): cv.use_id(light.AddressableLightState),

        cv.Required(CONF_CALIBRATION): sensor.sensor_schema(
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                cv.Schema(
                     {
                       cv.Optional(CONF_CALIBRATION_TIME, default="2s"): cv.All(
                                cv.positive_time_period_milliseconds,
                                cv.Range(max=TimePeriod(milliseconds=65535)),
                       ),
                     }
                ),
            ),
        cv.Required(CONF_TIME): sensor.sensor_schema(
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                cv.Schema(
                     {
                       cv.Optional(CONF_MIN_UP_TIME, default="10ms"): cv.All(
                                cv.positive_time_period_milliseconds,
                                cv.Range(max=TimePeriod(milliseconds=65535)),
                       ),
                       cv.Optional(CONF_MAX_UP_TIME, default="2000ms"): cv.All(
                                cv.positive_time_period_milliseconds,
                                cv.Range(max=TimePeriod(milliseconds=65535)),
                       ),
                       cv.Optional(CONF_LOW_OFF_TIME, default="10ms"): cv.All(
                                cv.positive_time_period_milliseconds,
                                cv.Range(max=TimePeriod(milliseconds=65535)),
                       ),
                     }
                ),
            ),
        cv.Required(CONF_AMPLITUDE): sensor.sensor_schema(
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        cv.Required(CONF_COUNT): sensor.sensor_schema(
                state_class=STATE_CLASS_MEASUREMENT,
            ),


    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    # var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)


    if isinstance(config[CONF_THRESHOLD], float):
        cg.add(var.set_upper_threshold(config[CONF_THRESHOLD]))
        cg.add(var.set_lower_threshold(config[CONF_THRESHOLD]))
    else:
        cg.add(var.set_upper_threshold(config[CONF_THRESHOLD][CONF_UPPER]))
        cg.add(var.set_lower_threshold(config[CONF_THRESHOLD][CONF_LOWER]))

    if CONF_ABSOLUTE in config:
        var.set_absolute_measurment(config[CONF_ABSOLUTE])

    if CONF_FEEDBACK_LIGHT_ID in config:
        light = await cg.get_variable(config[CONF_FEEDBACK_LIGHT_ID])

    if CONF_CALIBRATION in config:
        conf = config[CONF_CALIBRATION]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_calibration_sensor(sens))
        var.set_calibration_time(conf[CONF_CALIBRATION_TIME])

    if CONF_ABSOLUTE in config:
        var.set_absolute_delta(config[CONF_ABSOLUTE])

    if CONF_ANALOG in config:
        conf = config[CONF_ANALOG]
        sens = await cg.get_variable(conf[CONF_SENSOR_ID])
        cg.add(var.set_analog_sensor(sens))
        if CONF_ANALOG_FEEDBACK in conf:
            sens = await sensor.new_sensor(conf[CONF_ANALOG_FEEDBACK])
            cg.add(var.set_adc_sensor(sens))

    if CONF_TIME in config:
        conf = config[CONF_TIME]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_pulse_time_sensor(sens))
        cg.add(var.set_pulse_minimum_time(conf[CONF_MIN_UP_TIME]))
        cg.add(var.set_pulse_maximum_time(conf[CONF_MAX_UP_TIME]))
        cg.add(var.set_pulse_off_time(conf[CONF_LOW_OFF_TIME]))

    if CONF_AMPLITUDE in config:
        conf = config[CONF_AMPLITUDE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_pulse_amplitude_sensor(sens))

    if CONF_COUNT in config:
        conf = config[CONF_COUNT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_pulse_counter_sensor(sens))


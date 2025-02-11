import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import char_oled_base
from esphome.const import (
    CONF_DATA_PINS,
    CONF_ENABLE_PIN,
    CONF_RS_PIN,
    CONF_RW_PIN,
    CONF_ID,
    CONF_LAMBDA,
)

AUTO_LOAD = ["char_oled_base"]

char_oled_gpio_ns = cg.esphome_ns.namespace("char_oled_gpio")
GPIOOLEDDisplay = char_oled_gpio_ns.class_("GPIOOLEDDisplay", char_oled_base.OLEDDisplay)


def validate_pin_length(value):
    if len(value) != 4:
        raise cv.Invalid(
            f"OLED Displays can either operate in 4-pin mode, not {len(value)}-pin mode"
        )
    return value


CONFIG_SCHEMA = char_oled_base.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(GPIOOLEDDisplay),
        cv.Required(CONF_DATA_PINS): cv.All(
            [pins.gpio_output_pin_schema], validate_pin_length
        ),
        cv.Required(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_RS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_RW_PIN): pins.gpio_output_pin_schema,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await char_oled_base.setup_lcd_display(var, config)
    pins_ = []
    for conf in config[CONF_DATA_PINS]:
        pins_.append(await cg.gpio_pin_expression(conf))
    cg.add(var.set_data_pins(*pins_))
    enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
    cg.add(var.set_enable_pin(enable))

    rs = await cg.gpio_pin_expression(config[CONF_RS_PIN])
    cg.add(var.set_rs_pin(rs))

    if CONF_RW_PIN in config:
        rw = await cg.gpio_pin_expression(config[CONF_RW_PIN])
        cg.add(var.set_rw_pin(rw))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(GPIOOLEDDisplay.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))

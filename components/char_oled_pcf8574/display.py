import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import char_oled_base, i2c
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["char_oled_base"]

char_oled_pcf8574_ns = cg.esphome_ns.namespace("char_oled_pcf8574")
PCF8574OLEDDisplay = char_oled_pcf8574_ns.class_(
    "PCF8574OLEDDisplay", char_oled_base.OLEDDisplay, i2c.I2CDevice
)

CONFIG_SCHEMA = char_oled_base.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PCF8574OLEDDisplay),
    }
).extend(i2c.i2c_device_schema(0x3F))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await char_oled_base.setup_lcd_display(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(PCF8574OLEDDisplay.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))

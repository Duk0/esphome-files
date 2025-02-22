import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
from esphome.const import (
    CONF_DIMENSIONS,
    CONF_POSITION,
    CONF_DATA,
)

CONF_USER_CHARACTERS = "user_characters"

#CONF_VERSION = "version"
#CONF_CHARSET = "character_set"

char_oled_base_ns = cg.esphome_ns.namespace("char_oled_base")
OLEDDisplay = char_oled_base_ns.class_("OLEDDisplay", cg.PollingComponent)

#OLEDHardware = char_oled_base_ns.enum("OLEDHardware")
#OLEDHW_OPTIONS = {
#    "HW_V1": OLEDHardware.OLED_HW_V1,
#    "HW_V2": OLEDHardware.OLED_HW_V2,
#}
#OLEDCharacterSet = char_oled_base_ns.enum("OLEDCharacterSet")
#OLEDCHAR_OPTIONS = {
#    "JAPANESE": OLEDCharacterSet.OLED_JAPANESE,
#    "EUROPEAN_I": OLEDCharacterSet.OLED_EUROPEAN_I,
#    "RUSSIAN": OLEDCharacterSet.OLED_RUSSIAN,
#    "EUROPEAN_II": OLEDCharacterSet.OLED_EUROPEAN_II,
#}

def validate_lcd_dimensions(value):
    value = cv.dimensions(value)
    if value[0] > 0x40:
        raise cv.Invalid("OLED displays can't have more than 64 columns")
    if value[1] > 4:
        raise cv.Invalid("OLED displays can't have more than 4 rows")
    return value


def validate_user_characters(value):
    positions = set()
    for conf in value:
        if conf[CONF_POSITION] in positions:
            raise cv.Invalid(
                f"Duplicate user defined character at position {conf[CONF_POSITION]}"
            )
        positions.add(conf[CONF_POSITION])
    return value


LCD_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.Required(CONF_DIMENSIONS): validate_lcd_dimensions,
        cv.Optional(CONF_USER_CHARACTERS): cv.All(
            cv.ensure_list(
                cv.Schema(
                    {
                        cv.Required(CONF_POSITION): cv.int_range(min=0, max=7),
                        cv.Required(CONF_DATA): cv.All(
                            cv.ensure_list(cv.int_range(min=0, max=31)),
                            cv.Length(min=8, max=8),
                        ),
                    }
                ),
            ),
            cv.Length(max=8),
            validate_user_characters,
        ),
#        cv.Optional(CONF_VERSION, default="HW_V1"): cv.enum(
#            OLEDHW_OPTIONS, upper=True, space="_"
#        ),
#        cv.Optional(CONF_CHARSET, default="JAPANESE"): cv.enum(
#            OLEDCHAR_OPTIONS, upper=True, space="_"
#        ),
    }
).extend(cv.polling_component_schema("1s"))


async def setup_lcd_display(var, config):
    await display.register_display(var, config)
    cg.add(var.set_dimensions(config[CONF_DIMENSIONS][0], config[CONF_DIMENSIONS][1]))
    if CONF_USER_CHARACTERS in config:
        for usr in config[CONF_USER_CHARACTERS]:
            cg.add(var.set_user_defined_char(usr[CONF_POSITION], usr[CONF_DATA]))

#    cg.add(var.set_hw_version(config[CONF_VERSION]))
#    cg.add(var.set_character_set(config[CONF_CHARSET]))

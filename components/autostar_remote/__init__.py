import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL
from esphome.components import uart as uart_component
from esphome.components import text_sensor as text_sensor_component

AUTO = cg.esphome_ns.namespace('autostar_remote')
AutoStarRemote = AUTO.class_('AutoStarRemote', cg.PollingComponent, uart_component.UARTDevice)

CONF_UART_ID = 'uart_id'
CONF_LCD_TEXT_SENSOR_ID = 'lcd_text_sensor_id'
CONF_MAX_HOLD_SECONDS = 'max_hold_seconds'

# Schema: require uart_id and lcd_text_sensor_id and accept update_interval; max_hold_seconds optional
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AutoStarRemote),
    cv.Required(CONF_UART_ID): cv.use_id(uart_component.UARTComponent),
    cv.Required(CONF_LCD_TEXT_SENSOR_ID): cv.use_id(text_sensor_component.TextSensor),
    cv.Optional(CONF_UPDATE_INTERVAL, default='1000ms'): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_MAX_HOLD_SECONDS, default='10s'): cv.positive_time_period_seconds,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    uart_var = await cg.get_variable(config[CONF_UART_ID])
    lcd_var = await cg.get_variable(config[CONF_LCD_TEXT_SENSOR_ID])

    # Temporarily remove CONF_UPDATE_INTERVAL so register_component doesn't try to pass the
    # TimePeriod object directly into set_update_interval (which causes codegen errors).
    update_val = None
    if CONF_UPDATE_INTERVAL in config:
        update_val = config.pop(CONF_UPDATE_INTERVAL)

    var = cg.new_Pvariable(config[CONF_ID], uart_var, lcd_var)
    await cg.register_component(var, config)

    # If update_interval was provided, convert it to milliseconds (int) and add set_update_interval call.
    if update_val is not None:
    #    total_seconds_attr = getattr(update_val, "total_seconds", None)
    #    try:
    #        if total_seconds_attr is not None:
    #            secs = total_seconds_attr() if callable(total_seconds_attr) else float(total_seconds_attr)
    #        else:
    #            secs = float(update_val)
    #    except Exception:
    #        try:
    #            secs = int(update_val)
    #        except Exception:
    #            secs = 1.0
    #    ms = int(secs * 1000)
    #    cg.add(var.set_update_interval(ms))
        cg.add(var.set_update_interval(update_val))

    # Restore CONF_UPDATE_INTERVAL in config (optional)
    if update_val is not None:
        config[CONF_UPDATE_INTERVAL] = update_val

    # Compute max_hold_ms robustly from the config value (which can be a TimePeriod or numeric)
    max_hold = config.get(CONF_MAX_HOLD_SECONDS)
    ms = 10000
    if max_hold is not None:
        total_seconds_attr = getattr(max_hold, "total_seconds", None)
        try:
            if total_seconds_attr is not None:
                secs = total_seconds_attr() if callable(total_seconds_attr) else float(total_seconds_attr)
            else:
                secs = float(max_hold)
        except Exception:
            try:
                secs = int(max_hold)
            except Exception:
                secs = 10
        ms = int(secs * 1000)

    # Pass an integer (milliseconds) expression to the generated code
    cg.add(var.set_max_hold_ms(ms))
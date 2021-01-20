#include "opentherm_gw_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace opentherm {

static const char *TAG = "opentherm_gw.climate";

void OpenThermGWClimate::setup() {
#if 0
  this->sensor_->add_on_state_callback([this](float state) {
    this->current_temperature = state;
    // control may have changed, recompute
    this->compute_state_();
    // current temperature changed, publish state
    this->publish_state();
  });
  this->current_temperature = this->sensor_->state;

  // restore set points
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->to_call(this).perform();
  } else {
    // restore from defaults, change_away handles those for us
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->change_away_(false);
  }
#endif

  mOT.begin(std::bind(&OpenThermGWClimate::processRequest, this, std::placeholders::_1, std::placeholders::_2));
  sOT.begin(nullptr);
}

void OpenThermGWClimate::loop()
{
    mOT.loop();
}
void OpenThermGWClimate::control(const climate::ClimateCall &call) {
#if 0
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature_low().has_value())
    this->target_temperature_low = *call.get_target_temperature_low();
  if (call.get_target_temperature_high().has_value())
    this->target_temperature_high = *call.get_target_temperature_high();
  if (call.get_away().has_value())
    this->change_away_(*call.get_away());

  this->compute_state_();
  this->publish_state();
#endif
}
climate::ClimateTraits OpenThermGWClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_auto_mode(true);
//  traits.set_supports_cool_mode(this->supports_cool_);
  traits.set_supports_heat_mode(this->supports_heat_);
  traits.set_supports_two_point_target_temperature(true);
  traits.set_supports_away(this->supports_away_);
  traits.set_supports_action(true);
  return traits;
}
void OpenThermGWClimate::compute_state_() {
  if (this->mode != climate::CLIMATE_MODE_AUTO) {
    // in non-auto mode, switch directly to appropriate action
    //  - HEAT mode -> HEATING action
    //  - OFF mode -> OFF action (not IDLE!)
    this->switch_to_action_(static_cast<climate::ClimateAction>(this->mode));
    return;
  }
  if (isnan(this->current_temperature) || isnan(this->target_temperature_low) || isnan(this->target_temperature_high)) {
    // if any control parameters are nan, go to OFF action (not IDLE!)
    this->switch_to_action_(climate::CLIMATE_ACTION_OFF);
    return;
  }
  const bool too_cold = this->current_temperature < this->target_temperature_low;
  const bool too_hot = this->current_temperature > this->target_temperature_high;

  climate::ClimateAction target_action;
  if (too_cold) {
    // too cold -> enable heating if possible, else idle
    if (this->supports_heat_)
      target_action = climate::CLIMATE_ACTION_HEATING;
    else
      target_action = climate::CLIMATE_ACTION_IDLE;
  } else if (too_hot) {
      target_action = climate::CLIMATE_ACTION_IDLE;
  } else {
    // too hot -> in range
    if (this->supports_heat_) {
      // if supports heat, go to idle action
      target_action = climate::CLIMATE_ACTION_IDLE;
    } else {
      // else use current mode and don't change (hysteresis)
      target_action = this->action;
    }
  }

  this->switch_to_action_(target_action);
}
void OpenThermGWClimate::switch_to_action_(climate::ClimateAction action) {
  if (action == this->action)
    // already in target mode
    return;

  if ((action == climate::CLIMATE_ACTION_OFF && this->action == climate::CLIMATE_ACTION_IDLE) ||
      (action == climate::CLIMATE_ACTION_IDLE && this->action == climate::CLIMATE_ACTION_OFF)) {
    // switching from OFF to IDLE or vice-versa
    // these only have visual difference. OFF means user manually disabled,
    // IDLE means it's in auto mode but value is in target range.
    this->action = action;
    this->publish_state();
    return;
  }

  if (this->prev_trigger_ != nullptr) {
    this->prev_trigger_->stop_action();
    this->prev_trigger_ = nullptr;
  }
  Trigger<> *trig;
  switch (action) {
    case climate::CLIMATE_ACTION_OFF:
    case climate::CLIMATE_ACTION_IDLE:
      trig = this->idle_trigger_;
      break;
    case climate::CLIMATE_ACTION_HEATING:
      trig = this->heat_trigger_;
      break;
    default:
      trig = nullptr;
  }
  assert(trig != nullptr);
  trig->trigger();
  this->action = action;
  this->prev_trigger_ = trig;
  this->publish_state();
}
void OpenThermGWClimate::change_away_(bool away) {
  if (!away) {
    this->target_temperature_low = this->normal_config_.default_temperature_low;
    this->target_temperature_high = this->normal_config_.default_temperature_high;
  } else {
    this->target_temperature_low = this->away_config_.default_temperature_low;
    this->target_temperature_high = this->away_config_.default_temperature_high;
  }
  this->away = away;
}
void OpenThermGWClimate::set_normal_config(const OpenThermGWClimateTargetTempConfig &normal_config) {
  this->normal_config_ = normal_config;
}
void OpenThermGWClimate::set_away_config(const OpenThermGWClimateTargetTempConfig &away_config) {
  this->supports_away_ = true;
  this->away_config_ = away_config;
}

OpenThermGWClimate::OpenThermGWClimate(GPIOPin *m_pin_in, GPIOPin *m_pin_out, GPIOPin *s_pin_in, GPIOPin *s_pin_out)
    : idle_trigger_(new Trigger<>()),
      heat_trigger_(new Trigger<>()),
      mOT(m_pin_in, m_pin_out),
      sOT(s_pin_in, s_pin_out, true)
{
}

void OpenThermGWClimate::set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }
Trigger<> *OpenThermGWClimate::get_idle_trigger() const { return this->idle_trigger_; }
Trigger<> *OpenThermGWClimate::get_heat_trigger() const { return this->heat_trigger_; }
void OpenThermGWClimate::set_supports_heat(bool supports_heat) { this->supports_heat_ = supports_heat; }
void OpenThermGWClimate::dump_config() {
  LOG_CLIMATE("", "OpenTherm Gateway Climate", this);
  ESP_LOGCONFIG(TAG, "  Supports HEAT: %s", YESNO(this->supports_heat_));
  ESP_LOGCONFIG(TAG, "  Supports AWAY mode: %s", YESNO(this->supports_away_));
  ESP_LOGCONFIG(TAG, "  Default Target Temperature Low: %.1f°C", this->normal_config_.default_temperature_low);
  ESP_LOGCONFIG(TAG, "  Default Target Temperature High: %.1f°C", this->normal_config_.default_temperature_high);

  ESP_LOGCONFIG(TAG, "  Default Target Temperature High: %.1f°C", this->normal_config_.default_temperature_high);
}

OpenThermGWClimateTargetTempConfig::OpenThermGWClimateTargetTempConfig() = default;
OpenThermGWClimateTargetTempConfig::OpenThermGWClimateTargetTempConfig(float default_temperature_low,
                                                                 float default_temperature_high)
    : default_temperature_low(default_temperature_low),
      default_temperature_high(default_temperature_high)
{
}

void OpenThermGWClimate::processRequest(uint32_t request, OpenThermResponseStatus status) {

    // master/thermostat request
    OpenThermMessageID id = getDataID(request);
    uint16_t data = getUInt16(request);
    ESP_LOGD(TAG, "T %03d %04x", id, data);

    switch (id) {
      case MSG_DATE:
        process_Master_MSG_DATE(request);
        break;
      case MSG_DAY_TIME:
        process_Master_MSG_DAY_TIME(request);
        break;
      case MSG_YEAR:
        process_Master_MSG_YEAR(request);
        break;
      case MSG_COMMAND:
        process_Master_MSG_COMMAND(request);
        break;
      case MSG_MASTER_VERSION:
        process_Master_MSG_MASTER_VERSION(request);
        break;
      case MSG_M_CONFIG_M_MEMBERIDCODE:
        process_Master_MSG_M_CONFIG_M_MEMBERIDCODE(request);
        break;
      case MSG_OPENTHERM_VERSION_MASTER:
        process_Master_MSG_OPENTHERM_VERSION_MASTER(request);
        break;
      case MSG_STATUS:
        process_Master_MSG_STATUS(request);
        break;
      case MSG_TR:
        process_Master_MSG_TR(request);
        break;
      case MSG_TRSET:
        process_Master_MSG_TRSET(request);
        break;
      case MSG_TRSETCH2:
        process_Master_MSG_TRSETCH2(request);
        break;
      case MSG_TSET:
        process_Master_MSG_TSET(request);
        break;
      case MSG_TSETCH2:
        process_Master_MSG_TSETCH2(request);
        break;
      default:
        ESP_LOGD(TAG, "Request %d not handled!", id);
        break;
    }

    const uint32_t response = sOT.sendRequest(request);
    processResponse(response);
}

void OpenThermGWClimate::processResponse(const uint32_t response) {
    // slave/boiler response
    OpenThermMessageID id = getDataID(response);
    uint16_t data = getUInt16(response);
    ESP_LOGD(TAG, "B %03d %04x", id, data);
    switch (id) {
      case MSG_BURNER_STARTS:
        process_Slave_MSG_BURNER_STARTS(response);
        break;
      case MSG_CH_PRESSURE:
        process_Slave_MSG_CH_PRESSURE(response);
        break;
      case MSG_CH_PUMP_OPERATION_HOURS:
        process_Slave_MSG_CH_PUMP_OPERATION_HOURS(response);
        break;
      case MSG_CH_PUMP_STARTS:
        process_Slave_MSG_CH_PUMP_STARTS(response);
        break;
      case MSG_COMMAND:
        process_Slave_MSG_COMMAND(response);
        break;
      case MSG_DHW_BURNER_OPERATION_HOURS:
        process_Slave_MSG_DHW_BURNER_OPERATION_HOURS(response);
        break;
      case MSG_DHW_BURNER_STARTS:
        process_Slave_MSG_DHW_BURNER_STARTS(response);
        break;
      case MSG_DHW_FLOW_RATE:
        process_Slave_MSG_DHW_FLOW_RATE(response);
        break;
      case MSG_DHW_PUMP_VALVE_OPERATION_HOURS:
        process_Slave_MSG_DHW_PUMP_VALVE_OPERATION_HOURS(response);
        break;
      case MSG_DHW_PUMP_VALVE_STARTS:
        process_Slave_MSG_DHW_PUMP_VALVE_STARTS(response);
        break;
      case MSG_BURNER_OPERATION_HOURS:
        process_Slave_MSG_BURNER_OPERATION_HOURS(response);
        break;
      case MSG_OPENTHERM_VERSION_SLAVE:
        process_Slave_MSG_OPENTHERM_VERSION_SLAVE(response);
        break;
      case MSG_REL_MOD_LEVEL:
        process_Slave_MSG_REL_MOD_LEVEL(response);
        break;
      case MSG_REMOTE_OVERRIDE_FUNCTION:
        process_Slave_MSG_REMOTE_OVERRIDE_FUNCTION(response);
        break;
      case MSG_SLAVE_VERSION:
        process_Slave_MSG_SLAVE_VERSION(response);
        break;
      case MSG_STATUS:
        process_Slave_MSG_STATUS(response);
        break;
      case MSG_S_CONFIG_S_MEMBERIDCODE:
        process_Slave_MSG_S_CONFIG_S_MEMBERIDCODE(response);
        break;
      case MSG_TBOILER:
        process_Slave_MSG_TBOILER(response);
        break;
      case MSG_TCOLLECTOR:
        process_Slave_MSG_TCOLLECTOR(response);
        break;
      case MSG_TDHW2:
        process_Slave_MSG_TDHW2(response);
        break;
      case MSG_TDHW:
        process_Slave_MSG_TDHW(response);
        break;
      case MSG_TEXHAUST:
        process_Slave_MSG_TEXHAUST(response);
        break;
      case MSG_TFLOWCH2:
        process_Slave_MSG_TFLOWCH2(response);
        break;
      case MSG_TOUTSIDE:
        process_Slave_MSG_TOUTSIDE(response);
        break;
      case MSG_TRET:
        process_Slave_MSG_TRET(response);
        break;
      case MSG_TROVERRIDE:
        process_Slave_MSG_TROVERRIDE(response);
        break;
      case MSG_TSTORAGE:
        process_Slave_MSG_TSTORAGE(response);
        break;
      case MSG_DATE:
        process_Slave_MSG_DATE(response);
        break;
      case MSG_DAY_TIME:
        process_Slave_MSG_DAY_TIME(response);
        break;
      case MSG_YEAR:
        process_Slave_MSG_YEAR(response);
        break;
      case MSG_ASF_FLAGS_OEM_FAULT_CODE:
        process_Slave_MSG_ASF_FLAGS_OEM_FAULT_CODE(response);
        break;
      default:
        ESP_LOGD(TAG, "Response %d not handled!", id);
        break;
    }

    mOT.sendResponse(response);
}


/** Class 1 : Control and Status Information **/
// This group of data-items contains important control and status information relating to the slave and master.
// The slave status contains a mandatory fault-indication flag and there is an optional application-specific set of
// fault flags which relate to specific faults in boiler-related applications, and an OEM fault code whose meaning
// is unknown to the master but can be used for display purposes.

// #0: Status
// The gateway may want to manipulate the status message generated by the
// thermostat for two reasons: The domestic hot water enable option specified
// via a serial command may differ from what the thermostat sent. And if a
// control setpoint has been specified via a serial command, central heating
// mode must be enabled for the setpoint to have any effect.
// Also if the master status byte was changed before sending it to the
// boiler, the original master status must be restored before the message is
// returned to the thermostat, so it doesn't get confused
void OpenThermGWClimate::process_Master_MSG_STATUS(const uint32_t request) {
    uint8_t ub = getUBUInt8(request);

    // The CH enabled bit has priority over the Control Setpoint. The master can indicate that no CH demand is
    // required by putting the CH enabled bit = 0 (ie CH is disabled), even if the Control Setpoint is non-zero.
    bool master_ch_enabled      = ub & (1 << 0);
    bool master_dhw_enabled     = ub & (1 << 1);
    bool master_cooling_enabled = ub & (1 << 2);
    bool master_otc_enabled     = ub & (1 << 3);
    bool master_ch2_enabled     = ub & (1 << 4);

    //ESP_LOGD(TAG, "master_ch_enabled: %s", YESNO(master_ch_enabled));
    //ESP_LOGD(TAG, "master_dhw_enabled: %s", YESNO(master_dhw_enabled));
    //ESP_LOGD(TAG, "master_cooling_enabled: %s", YESNO(master_cooling_enabled));
    //ESP_LOGD(TAG, "master_otc_enabled: %s", YESNO(master_otc_enabled));
    //ESP_LOGD(TAG, "master_ch2_enabled: %s", YESNO(master_ch2_enabled));
}

void OpenThermGWClimate::process_Slave_MSG_STATUS(const uint32_t response) {
    uint8_t lb = getLBUInt8(response);

    bool slave_fault_indication = lb & (1 << 0);
    bool slave_ch_active        = lb & (1 << 1);
    bool slave_dhw_active       = lb & (1 << 2);
    bool slave_flame_on         = lb & (1 << 3);
    bool slave_cooling_active   = lb & (1 << 4);
    bool slave_ch2_active       = lb & (1 << 5);
    bool slave_diagnostic_event = lb & (1 << 6);

    //ESP_LOGD(TAG, "slave_fault_indication: %s", YESNO(slave_fault_indication));
    //ESP_LOGD(TAG, "slave_ch_active: %s", YESNO(slave_ch_active));
    //ESP_LOGD(TAG, "slave_dhw_active: %s", YESNO(slave_dhw_active));
    //ESP_LOGD(TAG, "slave_flame_on: %s", YESNO(slave_flame_on));
    //ESP_LOGD(TAG, "slave_cooling_active: %s", YESNO(slave_cooling_active));
    //ESP_LOGD(TAG, "slave_ch2_active: %s", YESNO(slave_ch2_active));
    //ESP_LOGD(TAG, "slave_diagnostic_event: %s", YESNO(slave_diagnostic_event));

    if (slave_fault_indication) {
      // Remember a fault was reported
      // ASF-flags/OEM-fault-code message
      // Schedule request for more information

    }

}


// #1: If a control setpoint has been specified by hass, the specified
// value is put into the message before sending it to the boiler.
// If the boiler indicates that the control setpoint is invalid, the value
// specified by the user is cleared.
void OpenThermGWClimate::process_Master_MSG_TSET(const uint32_t request) {
  // The master decides the actual range over which the control setpoint is defined. The default range is
  // assumed to be 0 to 100.
  /*if (temperature < 0.0f)
    temperature = 0.0f;
  if (temperature > 100.0f)
    temperature = 100.0f;*/

//    btfsc MsgResponse ;Request or response?
//    goto  ctrlsetptreturn
//    call  MandatoryID ;Prevent the DataID getting blacklisted
//    movfw controlsetpt1 ;Check if a setpoint has been specified
//    xorwf controlsetpt2,W
//    skpz
//    btfss byte1,4   ;WriteData request?
//    return      ;Continue with normal processing
//    movfw controlsetpt1 ;Fill in the specified setpoint
//    call  setbyte3
//    movfw controlsetpt2
//    goto  setbyte4
//
//ctrlsetptreturn btfss byte1,5
//    return      ;Setpoint acknowledged by boiler
//    btfsc byte1,4
//    return
//    tstf  controlsetpt1
//    skpnz
//    return
//    clrf  controlsetpt1 ;Zap the invalid value
//    clrf  controlsetpt2
//    return
}

// #5: Application-specific flags
// The Remeha iSense RF doesn't always report a room setpoint, but it does ask for
// the Application-specific flags quite regularly. If such a message is received
// after the remote setpoint clear command has been sent, we're going to assume
// the setpoint clear has been accepted.
void OpenThermGWClimate::process_Slave_MSG_ASF_FLAGS_OEM_FAULT_CODE(const uint32_t response) {
    // btfsc  MsgResponse ;Only interested in requests
    // goto WordResponse  ;Default handling for responses
    // btfsc  OverrideClr ;No pending clear ...
    // btfsc  OverrideWait  ;... or clear command has not been sent?
    // return     ;Do nothing
    // goto roomsetptclear  ;Consider setpoint clear accepted

    uint8_t ub = getUBUInt8(response);
    uint8_t oem_fault_code = getLBUInt8(response);

    bool master_service_request = ub & (1 << 0); // service not req’d, service required
    bool master_lockout_reset   = ub & (1 << 1); // remote reset disabled, rr enabled
    bool master_low_water_press = ub & (1 << 2); // no WP fault, water pressure fault
    bool master_gas_flame_fault = ub & (1 << 3); // no G/F fault, gas/flame fault
    bool master_air_press_fault = ub & (1 << 4); // no AP fault, air pressure fault
    bool master_water_over_temp = ub & (1 << 5); // no OvT fault, over-temperature fault
}

// #8: Control setpoint 2
void OpenThermGWClimate::process_Master_MSG_TSETCH2(const uint32_t request) {
  // The master decides the actual range over which the control setpoint is defined. The default range is
  // assumed to be 0 to 100.
  /*if (temperature < 0.0f)
    temperature = 0.0f;
  if (temperature > 100.0f)
    temperature = 100.0f;*/
}

// #115: OEM diagnostic code
void OpenThermGWClimate::process_Slave_MSG_OEM_DIAGNOSTIC_CODE(const uint32_t request) {
    uint16_t oem_diagnostic_code = getUInt16(request);
}

/** Class 2 : Configuration Information **/
// This group of data-items defines configuration information on both the slave and master sides. Each has a
// group of configuration flags (8 bits) and an MemberID code (1 byte). A valid Read Slave Configuration and
// Write Master Configuration message exchange is recommended before control and status information is
// transmitted.

// #2: Master configuration & Master MemberID code
void OpenThermGWClimate::process_Master_MSG_M_CONFIG_M_MEMBERIDCODE(const uint32_t response) {
    uint8_t master_configuration = getUBUInt8(response);

    // Remeha = 11
    uint8_t master_memberid_code = getLBUInt8(response);
}

// #3: Slave configuration & Slave MemberID code
void OpenThermGWClimate::process_Slave_MSG_S_CONFIG_S_MEMBERIDCODE(const uint32_t request) {
    uint8_t slave_configuration = getUBUInt8(request);
    uint8_t slave_member_id_code = getLBUInt8(request);

    bool master_dhw_present                   = slave_configuration & (1 << 0); // dhw not present, dhw is present
    bool master_control_type                  = slave_configuration & (1 << 1); // modulating, on/off
    bool master_cooling_config                = slave_configuration & (1 << 2); // cooling not supported, cooling supported
    bool master_dhw_config                    = slave_configuration & (1 << 3); // instantaneous or not-specified, storage tank
    bool master_low_off_pump_control_function = slave_configuration & (1 << 4); // allowed, not allowed
    bool master_ch2_present                   = slave_configuration & (1 << 5); // CH2 not present, CH2 present
}

// #124: OpenTherm version Master
void OpenThermGWClimate::process_Master_MSG_OPENTHERM_VERSION_MASTER(const uint32_t response) {
    uint16_t opentherm_version_master = getUInt16(response);
}

// #125: OpenTherm version Slave
void OpenThermGWClimate::process_Slave_MSG_OPENTHERM_VERSION_SLAVE(const uint32_t request) {
    uint16_t opentherm_version_slave = getUInt16(request);
}

// #126: Master product version number and type
void OpenThermGWClimate::process_Master_MSG_MASTER_VERSION(const uint32_t request) {
    uint8_t product_type = getUBUInt8(request);
    uint8_t product_version = getLBUInt8(request);
}

// #127: Slave product version number and type
void OpenThermGWClimate::process_Slave_MSG_SLAVE_VERSION(const uint32_t response) {
    uint8_t product_type = getUBUInt8(response);
    uint8_t product_version = getLBUInt8(response);
}

/* Class 3 : Remote Commands */
// This class of data represents commands sent by the master to the slave. There is a single data-id for a
// command “packet”, with the Command-Code embedded in the high-byte of the data-value field.

// #4: HB: Command-Code
void OpenThermGWClimate::process_Master_MSG_COMMAND(const uint32_t request) {
    uint8_t command_code = getUBUInt8(request);
}

// #4: Cmd-Response-Code
void OpenThermGWClimate::process_Slave_MSG_COMMAND(const uint32_t response) {
    uint8_t product_version = getLBUInt8(response);
}

/* Class 4 : Sensor and Informational Data */
// This group of data-items contains sensor data (temperatures, pressures etc.) and other informational data
// from one unit to the other.

// #16: Room Setpoint
void OpenThermGWClimate::process_Master_MSG_TRSET(const uint32_t request) {
    float room_setpoint = getFloat(request);
    ESP_LOGD(TAG, "room_setpoint: %f", room_setpoint);
}

// #17: Relative Modulation Level
void OpenThermGWClimate::process_Slave_MSG_REL_MOD_LEVEL(const uint32_t response) {
    float relative_modulation_level = getFloat(response);
}

// #18: Water pressure of the boiler CH circuit (bar)
void OpenThermGWClimate::process_Slave_MSG_CH_PRESSURE(const uint32_t response) {
    float ch_water_pressure = getFloat(response);
    ESP_LOGD(TAG, "ch_water_pressure: %f bar", ch_water_pressure);

}

// #19: Water flow rate through the DHW circuit (l/min)
void OpenThermGWClimate::process_Slave_MSG_DHW_FLOW_RATE(const uint32_t response) {
    float dhw_flow_rate = getFloat(response);
}

// #20: Day of Week & Time of Day
void OpenThermGWClimate::process_Master_MSG_DAY_TIME(const uint32_t msg) {
}
void OpenThermGWClimate::process_Slave_MSG_DAY_TIME(const uint32_t msg) {
}
// #21: Date
void OpenThermGWClimate::process_Master_MSG_DATE(const uint32_t msg) {
}
void OpenThermGWClimate::process_Slave_MSG_DATE(const uint32_t msg) {
}
// #22: Year
void OpenThermGWClimate::process_Master_MSG_YEAR(const uint32_t msg) {
}
void OpenThermGWClimate::process_Slave_MSG_YEAR(const uint32_t msg) {
}

// #23: Current room setpoint for 2nd CH circuit (°C)
void OpenThermGWClimate::process_Master_MSG_TRSETCH2(const uint32_t request) {
    float room_setpoint_ch2 = getFloat(request);
}

// #24: Current sensed room temperature (°C)
void OpenThermGWClimate::process_Master_MSG_TR(const uint32_t request) {
    float room_temperature = getFloat(request);
    ESP_LOGD(TAG, "room_temperature: %f", room_temperature);
}

// #25: Flow water temperature from boiler (°C)
void OpenThermGWClimate::process_Slave_MSG_TBOILER(const uint32_t response) {
    float boiler_water_temp = getFloat(response);
    ESP_LOGD(TAG, "boiler_water_temp: %f", boiler_water_temp);
}

// #26: Domestic hot water temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TDHW(const uint32_t response) {
    float dhw_temperature = getFloat(response);
    ESP_LOGD(TAG, "dhw_temperature: %f", dhw_temperature);
}

// #27: Outside air temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TOUTSIDE(const uint32_t response) {
    float outside_air_temperature = getFloat(response);
}

// #28: Return water temperature to boiler (°C)
void OpenThermGWClimate::process_Slave_MSG_TRET(const uint32_t response) {
    float return_water_temperature = getFloat(response);
}

// #29: Solar storage temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TSTORAGE(const uint32_t response) {
    float solar_storage_temperature = getFloat(response);
}

// #30: Solar collector temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TCOLLECTOR(const uint32_t response) {
    int16_t solar_storage_temperature = getInt16(response);
}

// #31: Flow water temperature of the second central
void OpenThermGWClimate::process_Slave_MSG_TFLOWCH2(const uint32_t response) {
    float flow_temperature_ch2 = getFloat(response);
}

// #32: Domestic hot water temperature 2 (°C)
void OpenThermGWClimate::process_Slave_MSG_TDHW2(const uint32_t response) {
    float dhw2_temperature = getFloat(response);
}

// #33: Exhaust temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TEXHAUST(const uint32_t response) {
    int16_t exhaust_temperature = getInt16(response);
}

// #116: Number of starts burner. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_BURNER_STARTS(const uint32_t response) {
    uint16_t burner_starts = getUInt16(response);
}

// #117: Number of starts CH pump. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_CH_PUMP_STARTS(const uint32_t response) {
    uint16_t ch_pump_starts = getUInt16(response);
}

// #118: Number of starts DHW pump/valve. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_PUMP_VALVE_STARTS(const uint32_t response) {
    uint16_t dhw_pump_valve_starts = getUInt16(response);
}

// #119: Number of starts burner in DHW mode. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_BURNER_STARTS(const uint32_t response) {
    uint16_t dhw_burner_starts = getUInt16(response);
}

// #120: Number of hours that burner is in operation (i.e. flame on). Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_BURNER_OPERATION_HOURS(const uint32_t response) {
    uint16_t burner_operation_hours = getUInt16(response);
}

// #121: Number of hours that CH pump has been running. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_CH_PUMP_OPERATION_HOURS(const uint32_t response) {
    uint16_t ch_pump_operation_hours = getUInt16(response);
}

// #122: Number of hours that DHW pump has been running or DHW valve has been opened. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_PUMP_VALVE_OPERATION_HOURS(const uint32_t response) {
    uint16_t dhw_pump_valve_operation_hours = getUInt16(response);
}

// #123: Number of hours that burner is in operation during DHW mode. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_BURNER_OPERATION_HOURS(const uint32_t response) {
    uint16_t dhw_burner_operation_hours = getUInt16(response);
}

/* Class 5 : Pre-Defined Remote Boiler Parameters */
// This group of data-items defines specific parameters of the slave device (setpoints, etc.) which may be
// available to the master device and may, or may not, be adjusted remotely. These parameters are
// prespecified in the protocol and are specifically related to boiler/room controller applications. There is a
// maximum of 8 remote boiler parameters. Each remote-boiler-parameter has a upper- and lower-bound (max
// and min values) which the master should read from the slave in order to make sure they are not set outside
// the valid range. If the slave does not support sending the upper- and lower-bounds, the master can apply
// default bounds as it chooses.

// The remote-parameter transfer-enable flags indicate which remote parameters are supported by the slave.
// The remote-parameter read/write flags indicate whether the master can only read the parameter from the
// slave, or whether it can also modify the parameter and write it back to the slave. An Unknown Data-Id
// response to a Read Remote-Parameter-Flags message indicates no support for remote-parameters
// (equivalent to all transfer-enable flags equal to zero). In these flag bytes bit 0 corresponds to
// remote-boilerparameter 1 and bit 7 to remote-boiler-parameter 8.

/* Class 8 : Control of Special Applications */

// #9: Remote override room setpoint
void OpenThermGWClimate::process_Slave_MSG_TROVERRIDE(const uint32_t response) {
    float remote_override_room_setpoint = getFloat(response);
    ESP_LOGD(TAG, "remote_override_room_setpoint: %f", remote_override_room_setpoint);
}

// #100:  Remote override function
void OpenThermGWClimate::process_Slave_MSG_REMOTE_OVERRIDE_FUNCTION(const uint32_t response) {
    uint8_t remote_override_function = getLBUInt8(response);
}


}  // namespace opentherm
}  // namespace esphome

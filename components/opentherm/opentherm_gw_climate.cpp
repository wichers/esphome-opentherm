#include "opentherm_gw_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace opentherm {

static const char *TAG = "opentherm_gw.climate";

OpenThermGWClimate::OpenThermGWClimate()
     : mOT(),
      sOT(true)
{
}

void OpenThermGWClimate::setup() {
  // restore set points
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->to_call(this).perform();
  } else {
    this->mode = climate::CLIMATE_MODE_AUTO;
  }

  mOT.setup(std::bind(&OpenThermGWClimate::processRequest, this, std::placeholders::_1, std::placeholders::_2));
  sOT.setup(nullptr);
}

void OpenThermGWClimate::loop()
{
    mOT.loop();
}

void OpenThermGWClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();
  //if (call.get_away().has_value())
  //    this->away = *call.get_away();

  // Set to 0 to pass along the value specified by the thermostat. To stop the boiler heating the house, 
  // set the control setpoint to some low value and clear the CH enable bit using the CH command.

  this->publish_state();
}

climate::ClimateTraits OpenThermGWClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
//traits.set_supports_auto_mode(true);
//  traits.set_supports_cool_mode(true);
  traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);
  traits.set_supported_presets({
      climate::CLIMATE_PRESET_HOME,
      climate::CLIMATE_PRESET_AWAY,
  });
  //traits.set_supports_action(true);
  return traits;
}

void OpenThermGWClimate::dump_config() {
  LOG_CLIMATE("", "OpenTherm Gateway Climate", this);
//  ESP_LOGCONFIG(TAG, "  Supports HEAT: %s", YESNO(this->supports_heat_));
}

void OpenThermGWClimate::processRequest(uint32_t request, OpenThermResponseStatus status) {

    // master/thermostat request
    OpenThermMessageID id = getDataID(request);
    uint16_t data = getUInt16(request);
    //ESP_LOGD(TAG, "T %03d %04x", id, data);

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
      case MSG_TDHWSET:
        process_Master_MSG_TDHWSET(request);
        break;
      case MSG_MAXTSET:
        process_Master_MSG_MAXTSET(request);
        break;
      case MSG_TSP_INDEX_TSP_VALUE:
        process_Master_MSG_TSP_INDEX_TSP_VALUE(request);
        break;
      case MSG_COOLING_CONTROL:
        process_Master_MSG_COOLING_CONTROL(request);
        break;
      case MSG_MAX_REL_MOD_LEVEL_SETTING:
        process_Master_MSG_MAX_REL_MOD_LEVEL_SETTING(request);
        break;
      default:
        //ESP_LOGD(TAG, "Request %d not handled!", id);
        break;
    }

    uint32_t response = sOT.sendRequest(request);
    status = sOT.getLastResponseStatus();
    processResponse(response, status);
}

void OpenThermGWClimate::processResponse(uint32_t &response, OpenThermResponseStatus status) {
    
    // slave/boiler response
    OpenThermMessageID id = getDataID(response);
    uint16_t data = getUInt16(response);
    //ESP_LOGD(TAG, "B %03d %04x", id, data);
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
      case MSG_RBP_FLAGS:
        process_Slave_MSG_RBP_FLAGS(response);
        break;
      case MSG_TDHWSET_UB_LB:
        process_Slave_MSG_TDHWSET_UB_LB(response);
        break;
      case MSG_MAXTSET_UB_LB:
        process_Slave_MSG_MAXTSET_UB_LB(response);
        break;
      case MSG_TDHWSET:
        process_Slave_MSG_TDHWSET(response);
        break;
      case MSG_MAXTSET:
        process_Slave_MSG_MAXTSET(response);
        break;
      case MSG_TSP:
        process_Slave_MSG_TSP(response);
        break;
      case MSG_TSP_INDEX_TSP_VALUE:
        process_Slave_MSG_TSP_INDEX_TSP_VALUE(response);
        break;
      case MSG_FHB_SIZE:
        process_Slave_MSG_FHB_SIZE(response);
        break;
      case MSG_FHB_INDEX_FHB_VALUE:
        process_Slave_MSG_FHB_INDEX_FHB_VALUE(response);
        break;
      case MSG_MAX_CAPACITY_MIN_MOD_LEVEL:
        process_Slave_MSG_MAX_CAPACITY_MIN_MOD_LEVEL(response);
        break;
      case MSG_OEM_DIAGNOSTIC_CODE:
        process_Slave_MSG_OEM_DIAGNOSTIC_CODE(response);
        break;
      default:
        //ESP_LOGD(TAG, "Response %d not handled!", id);
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
void OpenThermGWClimate::process_Master_MSG_STATUS(uint32_t &request) {
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

    //if (this->away) {
      
    //}
}

void OpenThermGWClimate::process_Slave_MSG_STATUS(uint32_t &response) {
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

    if (this->fault_indication != nullptr) {
      this->fault_indication->publish_state(slave_fault_indication);
    }
    if (this->ch_active != nullptr) {
      this->ch_active->publish_state(slave_ch_active);
    }
    if (this->dhw_active != nullptr) {
      this->dhw_active->publish_state(slave_dhw_active);
    }
    if (this->flame_on != nullptr) {
      this->flame_on->publish_state(slave_flame_on);
    }
    if (this->cooling_active != nullptr) {
      this->cooling_active->publish_state(slave_cooling_active);
    }
    if (this->ch2_active != nullptr) {
      this->ch2_active->publish_state(slave_ch2_active);
    }
    if (this->diagnostic_event != nullptr) {
      this->diagnostic_event->publish_state(slave_diagnostic_event);
    }

    if (slave_fault_indication) {
      // Remember a fault was reported
      // ASF-flags/OEM-fault-code message
      // Schedule request for more information

    }
}

// #1: Control setpoint ie CH water temperature setpoint (°C)
void OpenThermGWClimate::process_Master_MSG_TSET(uint32_t &request) {
    float control_setpoint = getFloat(request);
    ESP_LOGD(TAG, "CH water temperature setpoint (°C): %f", control_setpoint);
    if (control_setpoint != this->target_temperature) {
      //request = modifyMsgData(request, temperatureToData(this->target_temperature));
    }
}

// #5: Application-specific flags
void OpenThermGWClimate::process_Slave_MSG_ASF_FLAGS_OEM_FAULT_CODE(uint32_t &response) {
    uint8_t ub = getUBUInt8(response);
    uint8_t oem_fault_code = getLBUInt8(response);

    bool slave_service_request = ub & (1 << 0); // service not req’d, service required
    bool slave_lockout_reset   = ub & (1 << 1); // remote reset disabled, rr enabled
    bool slave_low_water_press = ub & (1 << 2); // no WP fault, water pressure fault
    bool slave_gas_flame_fault = ub & (1 << 3); // no G/F fault, gas/flame fault
    bool slave_air_press_fault = ub & (1 << 4); // no AP fault, air pressure fault
    bool slave_water_over_temp = ub & (1 << 5); // no OvT fault, over-temperature fault
}

// #8: Control setpoint for 2nd CH circuit (°C)
void OpenThermGWClimate::process_Master_MSG_TSETCH2(uint32_t &request) {
  // The master decides the actual range over which the control setpoint is defined. The default range is
  // assumed to be 0 to 100.
  /*if (temperature < 0.0f)
    temperature = 0.0f;
  if (temperature > 100.0f)
    temperature = 100.0f;*/
    float control_setpoint = getFloat(request);
    ESP_LOGD(TAG, "Control setpoint for 2nd CH circuit (°C): %f", control_setpoint);
}

// #115: OEM diagnostic code
void OpenThermGWClimate::process_Slave_MSG_OEM_DIAGNOSTIC_CODE(uint32_t &request) {
    uint16_t oem_diagnostic_code = getUInt16(request);
    ESP_LOGD(TAG, "oem_diagnostic_code: %d", oem_diagnostic_code);
}

/** Class 2 : Configuration Information **/
// This group of data-items defines configuration information on both the slave and master sides. Each has a
// group of configuration flags (8 bits) and an MemberID code (1 byte). A valid Read Slave Configuration and
// Write Master Configuration message exchange is recommended before control and status information is
// transmitted.

// #2: Master configuration & Master MemberID code
void OpenThermGWClimate::process_Master_MSG_M_CONFIG_M_MEMBERIDCODE(uint32_t &response) {
    uint8_t master_configuration = getUBUInt8(response);

    // Remeha = 11
    uint8_t master_memberid_code = getLBUInt8(response);
    ESP_LOGD(TAG, "master_memberid_code: %d", master_memberid_code);
}

// #3: Slave configuration & Slave MemberID code
void OpenThermGWClimate::process_Slave_MSG_S_CONFIG_S_MEMBERIDCODE(uint32_t &request) {
    uint8_t slave_configuration = getUBUInt8(request);
    uint8_t slave_member_id_code = getLBUInt8(request);

    bool slave_dhw_present                   = slave_configuration & (1 << 0); // dhw not present, dhw is present
    bool slave_control_type                  = slave_configuration & (1 << 1); // modulating, on/off
    bool slave_cooling_config                = slave_configuration & (1 << 2); // cooling not supported, cooling supported
    bool slave_dhw_config                    = slave_configuration & (1 << 3); // instantaneous or not-specified, storage tank
    bool slave_low_off_pump_control_function = slave_configuration & (1 << 4); // allowed, not allowed
    bool slave_ch2_present                   = slave_configuration & (1 << 5); // CH2 not present, CH2 present

    ESP_LOGD(TAG, "slave_dhw_present: %s", YESNO(slave_dhw_present));
    ESP_LOGD(TAG, "slave_control_type: %s", YESNO(slave_control_type));
    ESP_LOGD(TAG, "slave_cooling_config: %s", YESNO(slave_cooling_config));
    ESP_LOGD(TAG, "slave_dhw_config: %s", YESNO(slave_dhw_config));
    ESP_LOGD(TAG, "slave_low_off_pump_control_function: %s", YESNO(slave_low_off_pump_control_function));
    ESP_LOGD(TAG, "slave_ch2_present: %s", YESNO(slave_ch2_present));
}

// #124: OpenTherm version Master
void OpenThermGWClimate::process_Master_MSG_OPENTHERM_VERSION_MASTER(uint32_t &response) {
    uint16_t opentherm_version_master = getUInt16(response);
    ESP_LOGD(TAG, "opentherm_version_master: %d", opentherm_version_master);
}

// #125: OpenTherm version Slave
void OpenThermGWClimate::process_Slave_MSG_OPENTHERM_VERSION_SLAVE(uint32_t &request) {
    uint16_t opentherm_version_slave = getUInt16(request);
    ESP_LOGD(TAG, "opentherm_version_slave: %d", opentherm_version_slave);
}

// #126: Master product version number and type
void OpenThermGWClimate::process_Master_MSG_MASTER_VERSION(uint32_t &request) {
    uint8_t product_type = getUBUInt8(request);
    uint8_t product_version = getLBUInt8(request);
    ESP_LOGD(TAG, "master product type/version: %d/%d", product_type, product_version);
}

// #127: Slave product version number and type
void OpenThermGWClimate::process_Slave_MSG_SLAVE_VERSION(uint32_t &response) {
    uint8_t product_type = getUBUInt8(response);
    uint8_t product_version = getLBUInt8(response);
    ESP_LOGD(TAG, "slave product type/version: %d/%d", product_type, product_version);
}

/* Class 3 : Remote Commands */
// This class of data represents commands sent by the master to the slave. There is a single data-id for a
// command “packet”, with the Command-Code embedded in the high-byte of the data-value field.

// #4: HB: Command-Code
void OpenThermGWClimate::process_Master_MSG_COMMAND(uint32_t &request) {
    uint8_t command_code = getUBUInt8(request);
    ESP_LOGD(TAG, "master command code: %d", command_code);
}

// #4: Cmd-Response-Code
void OpenThermGWClimate::process_Slave_MSG_COMMAND(uint32_t &response) {
    uint8_t product_version = getLBUInt8(response);
    ESP_LOGD(TAG, "slave product_version: %d", product_version);
}

/* Class 4 : Sensor and Informational Data */
// This group of data-items contains sensor data (temperatures, pressures etc.) and other informational data
// from one unit to the other.

// #16: Room Setpoint
void OpenThermGWClimate::process_Master_MSG_TRSET(uint32_t &request) {
    float room_setpoint = getFloat(request);
    ESP_LOGD(TAG, "room_setpoint: %f", room_setpoint);
    if (this->target_temperature != room_setpoint) {
      this->target_temperature = room_setpoint;
      this->publish_state();
    }
}

// #17: Relative Modulation Level
void OpenThermGWClimate::process_Slave_MSG_REL_MOD_LEVEL(uint32_t &response) {
    float relative_modulation_level = getFloat(response);
    ESP_LOGD(TAG, "relative_modulation_level: %f", relative_modulation_level);
    if (this->relative_modulation_level != nullptr) {
      this->relative_modulation_level->publish_state(relative_modulation_level);
    }
}

// #18: Water pressure of the boiler CH circuit (bar)
void OpenThermGWClimate::process_Slave_MSG_CH_PRESSURE(uint32_t &response) {
    float ch_water_pressure = getFloat(response);
    ESP_LOGD(TAG, "ch_water_pressure: %f bar", ch_water_pressure);
    if (this->ch_water_pressure != nullptr) {
      this->ch_water_pressure->publish_state(ch_water_pressure);
    }
}

// #19: Water flow rate through the DHW circuit (l/min)
void OpenThermGWClimate::process_Slave_MSG_DHW_FLOW_RATE(uint32_t &response) {
    float dhw_flow_rate = getFloat(response);
    ESP_LOGD(TAG, "dhw_flow_rate: %f l/min", dhw_flow_rate);
    if (this->dhw_flow_rate != nullptr) {
      this->dhw_flow_rate->publish_state(dhw_flow_rate);
    }
}

// #20: Day of Week & Time of Day
void OpenThermGWClimate::process_Master_MSG_DAY_TIME(uint32_t &msg) {
    uint8_t ub = getUBUInt8(msg);
    uint8_t minutes = getLBUInt8(msg);
    uint8_t day_of_week = (ub >> 5) & 7;
    uint8_t hours = (ub & 0x1f);
}

void OpenThermGWClimate::process_Slave_MSG_DAY_TIME(uint32_t &msg) {
    uint8_t ub = getUBUInt8(msg);
    uint8_t minutes = getLBUInt8(msg);
    uint8_t day_of_week = (ub >> 5) & 7;
    uint8_t hours = (ub & 0x1f);
}

// #21: Date
void OpenThermGWClimate::process_Master_MSG_DATE(uint32_t &msg) {
    uint8_t month = getUBUInt8(msg);
    uint8_t day_of_month = getLBUInt8(msg);
}

void OpenThermGWClimate::process_Slave_MSG_DATE(uint32_t &msg) {
    uint8_t month = getUBUInt8(msg);
    uint8_t day_of_month = getLBUInt8(msg);
}

// #22: Year
void OpenThermGWClimate::process_Master_MSG_YEAR(uint32_t &msg) {
    uint16_t year = getUInt16(msg);
}

void OpenThermGWClimate::process_Slave_MSG_YEAR(uint32_t &msg) {
    uint16_t year = getUInt16(msg);
}

// #23: Current room setpoint for 2nd CH circuit (°C)
void OpenThermGWClimate::process_Master_MSG_TRSETCH2(uint32_t &request) {
    float room_setpoint_ch2 = getFloat(request);
    ESP_LOGD(TAG, "room_setpoint_ch2: %f", room_setpoint_ch2);
}

// #24: Current sensed room temperature (°C)
void OpenThermGWClimate::process_Master_MSG_TR(uint32_t &request) {
    float room_temperature = getFloat(request);
    ESP_LOGD(TAG, "room_temperature: %f", room_temperature);
    if (this->current_temperature != room_temperature) {
      this->current_temperature = room_temperature;
      this->publish_state();
    }
}

// #25: Flow water temperature from boiler (°C)
void OpenThermGWClimate::process_Slave_MSG_TBOILER(uint32_t &response) {
    float boiler_water_temp = getFloat(response);
    ESP_LOGD(TAG, "boiler_water_temp: %f", boiler_water_temp);
    if (this->boiler_water_temp != nullptr) {
      this->boiler_water_temp->publish_state(boiler_water_temp);
    }
}

// #26: Domestic hot water temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TDHW(uint32_t &response) {
    float dhw_temperature = getFloat(response);
    ESP_LOGD(TAG, "dhw_temperature: %f", dhw_temperature);
    if (this->dhw_temperature != nullptr) {
      this->dhw_temperature->publish_state(dhw_temperature);
    }
}

// #27: Outside air temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TOUTSIDE(uint32_t &response) {
    float outside_air_temperature = getFloat(response);
    ESP_LOGD(TAG, "outside_air_temperature: %f", outside_air_temperature);
    if (this->outside_air_temperature != nullptr) {
      this->outside_air_temperature->publish_state(outside_air_temperature);
    }
}

// #28: Return water temperature to boiler (°C)
void OpenThermGWClimate::process_Slave_MSG_TRET(uint32_t &response) {
    float return_water_temperature = getFloat(response);
    ESP_LOGD(TAG, "return_water_temperature: %f", return_water_temperature);
    if (this->return_water_temperature != nullptr) {
      this->return_water_temperature->publish_state(return_water_temperature);
    }
}

// #29: Solar storage temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TSTORAGE(uint32_t &response) {
    float solar_storage_temperature = getFloat(response);
    ESP_LOGD(TAG, "solar_storage_temperature: %f", solar_storage_temperature);
    if (this->solar_storage_temperature != nullptr) {
      this->solar_storage_temperature->publish_state(solar_storage_temperature);
    }
}

// #30: Solar collector temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TCOLLECTOR(uint32_t &response) {
    int16_t solar_collector_temperature = getInt16(response);
    ESP_LOGD(TAG, "solar_collector_temperature: %d", solar_collector_temperature);
    if (this->solar_collector_temperature != nullptr) {
      this->solar_collector_temperature->publish_state(solar_collector_temperature);
    }
}

// #31: Flow water temperature of the second central
void OpenThermGWClimate::process_Slave_MSG_TFLOWCH2(uint32_t &response) {
    float flow_temperature_ch2 = getFloat(response);
    ESP_LOGD(TAG, "flow_temperature_ch2: %f", flow_temperature_ch2);
    if (this->flow_temperature_ch2 != nullptr) {
      this->flow_temperature_ch2->publish_state(flow_temperature_ch2);
    }
}

// #32: Domestic hot water temperature 2 (°C)
void OpenThermGWClimate::process_Slave_MSG_TDHW2(uint32_t &response) {
    float dhw2_temperature = getFloat(response);
    ESP_LOGD(TAG, "dhw2_temperature: %f", dhw2_temperature);
    if (this->dhw2_temperature != nullptr) {
      this->dhw2_temperature->publish_state(dhw2_temperature);
    }
}

// #33: Exhaust temperature (°C)
void OpenThermGWClimate::process_Slave_MSG_TEXHAUST(uint32_t &response) {
    int16_t exhaust_temperature = getInt16(response);
    ESP_LOGD(TAG, "exhaust_temperature: %d", exhaust_temperature);
    if (this->exhaust_temperature != nullptr) {
      this->exhaust_temperature->publish_state(exhaust_temperature);
    }
}

// #116: Number of starts burner. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_BURNER_STARTS(uint32_t &response) {
    uint16_t burner_starts = getUInt16(response);
    ESP_LOGD(TAG, "burner_starts: %d", burner_starts);
    if (this->burner_starts != nullptr) {
      this->burner_starts->publish_state(burner_starts);
    }
}

// #117: Number of starts CH pump. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_CH_PUMP_STARTS(uint32_t &response) {
    uint16_t ch_pump_starts = getUInt16(response);
    ESP_LOGD(TAG, "ch_pump_starts: %d", ch_pump_starts);
    if (this->ch_pump_starts != nullptr) {
      this->ch_pump_starts->publish_state(ch_pump_starts);
    }
}

// #118: Number of starts DHW pump/valve. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_PUMP_VALVE_STARTS(uint32_t &response) {
    uint16_t dhw_pump_valve_starts = getUInt16(response);
    ESP_LOGD(TAG, "dhw_pump_valve_starts: %d", dhw_pump_valve_starts);
    if (this->dhw_pump_valve_starts != nullptr) {
      this->dhw_pump_valve_starts->publish_state(dhw_pump_valve_starts);
    }
}

// #119: Number of starts burner in DHW mode. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_BURNER_STARTS(uint32_t &response) {
    uint16_t dhw_burner_starts = getUInt16(response);
    ESP_LOGD(TAG, "dhw_burner_starts: %d", dhw_burner_starts);
    if (this->dhw_burner_starts != nullptr) {
      this->dhw_burner_starts->publish_state(dhw_burner_starts);
    }
}

// #120: Number of hours that burner is in operation (i.e. flame on). Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_BURNER_OPERATION_HOURS(uint32_t &response) {
    uint16_t burner_operation_hours = getUInt16(response);
    ESP_LOGD(TAG, "burner_operation_hours: %d", burner_operation_hours);
    if (this->burner_operation_hours != nullptr) {
      this->burner_operation_hours->publish_state(burner_operation_hours);
    }
}

// #121: Number of hours that CH pump has been running. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_CH_PUMP_OPERATION_HOURS(uint32_t &response) {
    uint16_t ch_pump_operation_hours = getUInt16(response);
    ESP_LOGD(TAG, "ch_pump_operation_hours: %d", ch_pump_operation_hours);
    if (this->ch_pump_operation_hours != nullptr) {
      this->ch_pump_operation_hours->publish_state(ch_pump_operation_hours);
    }
}

// #122: Number of hours that DHW pump has been running or DHW valve has been opened. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_PUMP_VALVE_OPERATION_HOURS(uint32_t &response) {
    uint16_t dhw_pump_valve_operation_hours = getUInt16(response);
    ESP_LOGD(TAG, "dhw_pump_valve_operation_hours: %d", dhw_pump_valve_operation_hours);
    if (this->dhw_pump_valve_operation_hours != nullptr) {
      this->dhw_pump_valve_operation_hours->publish_state(dhw_pump_valve_operation_hours);
    }
}

// #123: Number of hours that burner is in operation during DHW mode. Reset by writing zero is optional for slave.
void OpenThermGWClimate::process_Slave_MSG_DHW_BURNER_OPERATION_HOURS(uint32_t &response) {
    uint16_t dhw_burner_operation_hours = getUInt16(response);
    ESP_LOGD(TAG, "dhw_burner_operation_hours: %d", dhw_burner_operation_hours);
    if (this->dhw_burner_operation_hours != nullptr) {
      this->dhw_burner_operation_hours->publish_state(dhw_burner_operation_hours);
    }
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
// (equivalent to all transfer-enable flags equal to zero). In these flag bytes bit 0 corresponds to remote-boiler
// parameter 1 and bit 7 to remote-boiler-parameter 8.

// #6: Remote-parameter
void OpenThermGWClimate::process_Slave_MSG_RBP_FLAGS(uint32_t &response) {
    uint8_t ub = getUBUInt8(response);
    uint8_t lb = getLBUInt8(response);

    bool dhw_setpoint_enabled = ub & (1 << 0); // DHW setpoint
    bool max_ch_setpoint_enabled = ub & (1 << 1); // max CHsetpoint

    bool dhw_setpoint_readonly = lb & (1 << 0); // DHW setpoint read-only?
    bool max_ch_setpoint_readonly = lb & (1 << 1); // max CHsetpoint read-only?

    ESP_LOGD(TAG, "slave dhw enabled: %d, max_ch_setpoint_enabled: %d", dhw_setpoint_enabled, max_ch_setpoint_enabled);
}

// #48: DHW setpoint upper & lower bounds for adjustment (°C)
void OpenThermGWClimate::process_Slave_MSG_TDHWSET_UB_LB(uint32_t &response) {
    int8_t dhw_setpoint_ub = getUBInt8(response); // Upper bound for adjustment of DHW setpoint (°C)
    int8_t dhw_setpoint_lb = getLBInt8(response); // Lower bound for adjustment of DHW setpoint (°C)
    ESP_LOGD(TAG, "dhw_setpoint_ub: %d, dhw_setpoint_lb: %d", dhw_setpoint_ub, dhw_setpoint_lb);
}

// #49: Max CH water setpoint upper & lower bounds for adjustment (°C)
void OpenThermGWClimate::process_Slave_MSG_MAXTSET_UB_LB(uint32_t &response) {
    int8_t max_ch_setpoint_ub = getUBInt8(response); // Upper bound for adjustment of maxCHsetp (°C)
    int8_t max_ch_setpoint_lb = getLBInt8(response); // Lower bound for adjustment of maxCHsetp (°C)
    ESP_LOGD(TAG, "max_ch_setpoint_ub: %d, max_ch_setpoint_lb: %d", max_ch_setpoint_ub, max_ch_setpoint_lb);
}

// #56: DHW setpoint (°C) (Remote parameter 1)
void OpenThermGWClimate::process_Master_MSG_TDHWSET(uint32_t &request) {
    float dhw_setpoint = getFloat(request); // Domestic hot water temperature setpoint (°C)
    ESP_LOGD(TAG, "master dhw_setpoint: %f", dhw_setpoint);
}

// #56: DHW setpoint (°C) (Remote parameter 1)
void OpenThermGWClimate::process_Slave_MSG_TDHWSET(uint32_t &request) {
    float dhw_setpoint = getFloat(request); // Domestic hot water temperature setpoint (°C)
    ESP_LOGD(TAG, "slave dhw_setpoint: %f", dhw_setpoint);
}

// #57: Current room setpoint for 2nd CH circuit (°C)
void OpenThermGWClimate::process_Master_MSG_MAXTSET(uint32_t &request) {
    float max_ch_water_setpoint = getFloat(request); //  Maximum allowable CH water setpoint (°C)
    ESP_LOGD(TAG, "master max_ch_water_setpoint: %f", max_ch_water_setpoint);
}

// #57: Current room setpoint for 2nd CH circuit (°C)
void OpenThermGWClimate::process_Slave_MSG_MAXTSET(uint32_t &request) {
    float max_ch_water_setpoint = getFloat(request); //  Maximum allowable CH water setpoint (°C)
    ESP_LOGD(TAG, "slave max_ch_water_setpoint: %f", max_ch_water_setpoint);
}

/* Class 6 : Transparent Slave Parameters */
// This group of data-items defines parameters of the slave device which may (or may not) be remotely set by
// the master device. These parameters are not pre-specified in the protocol and are “transparent” to the master
// in the sense that it has no knowledge about their application meaning.

// The first data-item (id=10) allows the master to read the number of transparent-slave-parameters supported
// by the slave. The second data-item (ID=11) allows the master to read and write individual transparent-slave
// parameters from/to the slave.

// #10: Number of Transparent-Slave-Parameters supported by slave
void OpenThermGWClimate::process_Slave_MSG_TSP(uint32_t &response) {
    uint8_t number_of_tsp = getUBUInt8(response); // Number of transparent-slave-parameter supported by the slave device.
    ESP_LOGD(TAG, "number_of_tsp: %d", number_of_tsp);
}

// #11: Index number / Value of referred-to transparent slave parameter.
void OpenThermGWClimate::process_Master_MSG_TSP_INDEX_TSP_VALUE(uint32_t &response) {
    uint8_t tsp_index_no = getUBUInt8(response); // Index number of following TSP
    uint8_t tsp_value = getLBUInt8(response); // Value of above referenced TSP
    ESP_LOGD(TAG, "tsp_index_no: %d, tsp_index_no: %d", tsp_index_no, tsp_index_no);
}

// #11: Index number / Value of referred-to transparent slave parameter.
void OpenThermGWClimate::process_Slave_MSG_TSP_INDEX_TSP_VALUE(uint32_t &response) {
    uint8_t tsp_index_no = getUBUInt8(response); // Index number of following TSP
    uint8_t tsp_value = getLBUInt8(response); // Value of above referenced TSP
    ESP_LOGD(TAG, "tsp_index_no: %d, tsp_index_no: %d", tsp_index_no, tsp_index_no);
}

/* Class 7 : Fault History Data */
// This group of data-items contains information relating to the past fault condition of the slave device.

// #12: Size of Fault-History-Buffer supported by slave
void OpenThermGWClimate::process_Slave_MSG_FHB_SIZE(uint32_t &response) {
    uint8_t size_of_fault_buffer = getUBUInt8(response); // The size of the fault history buffer.
    ESP_LOGD(TAG, "size_of_fault_buffer: %d", size_of_fault_buffer);
}

// #13: Index number / Value of referred-to fault-history buffer entry.
void OpenThermGWClimate::process_Slave_MSG_FHB_INDEX_FHB_VALUE(uint32_t &response) {
    uint8_t fhb_entry_index_no = getUBUInt8(response); // Index number of following Fault Buffer entry
    uint8_t fhb_entry_value = getLBUInt8(response); // Value of above referenced Fault Buffer entry
    ESP_LOGD(TAG, "fhb_entry_index_no: %d, fhb_entry_value: %d", fhb_entry_index_no, fhb_entry_value);
}



// The remote-parameter transfer-enable flags indicate which remote parameters are supported by the slave.
// The remote-parameter read/write flags indicate whether the master can only read the parameter from the
// slave, or whether it can also modify the parameter and write it back to the slave. An Unknown Data-Id
// response to a Read Remote-Parameter-Flags message indicates no support for remote-parameters
// (equivalent to all transfer-enable flags equal to zero). In these flag bytes bit 0 corresponds to
// remote-boilerparameter 1 and bit 7 to remote-boiler-parameter 8.

/* Class 8 : Control of Special Applications */

// #7: Signal for cooling plant.
// The cooling control signal is to be used for cooling applications. First the master should determine if the slave
// supports cooling by reading the slave configuration. Then the master can use the cooling control signal and
// the cooling-enable flag (status) to control the cooling plant. The status of the cooling plant can be read from
// the slave cooling status bit.
void OpenThermGWClimate::process_Master_MSG_COOLING_CONTROL(uint32_t &request) {
    float cooling_control_signal = getFloat(request); //  Cooling control signal (%)
    ESP_LOGD(TAG, "cooling_control_signal: %f", cooling_control_signal);
}

// The boiler capacity level setting is to be used for boiler sequencer applications. The control setpoint should
// be set to maximum, and then the capacity level setting throttled back to the required value. The default value
// in the slave device should be 100% (ie no throttling back of the capacity). The master can read the maximum
// boiler capacity and minimum modulation levels from the slave if it supports these.

// #14: Maximum relative modulation level setting
void OpenThermGWClimate::process_Master_MSG_MAX_REL_MOD_LEVEL_SETTING(uint32_t &request) {
    // Maximum relative boiler modulation level setting for sequencer and off-low & pump control applications.
    float max_rel_mod_level = getFloat(request);
    if (this->max_relative_modulation_level.has_value()) {
      max_rel_mod_level = this->max_relative_modulation_level.value();
      request = modifyMsgData(request, max_rel_mod_level);
    }
    ESP_LOGD(TAG, "max_rel_mod_level: %f", max_rel_mod_level);
}

// #15: Maximum boiler capacity (kW) / Minimum boiler modulation level(%)
void OpenThermGWClimate::process_Slave_MSG_MAX_CAPACITY_MIN_MOD_LEVEL(uint32_t &response) {
    uint8_t max_boiler_capacity = getUBUInt8(response); // 0..255kW
    uint8_t min_modulation_level = getLBUInt8(response); // 0..100% expressed as a percentage of the maximum capacity.
    ESP_LOGD(TAG, "max_boiler_capacity: %d, min_modulation_level: %d", max_boiler_capacity, min_modulation_level);
}

// There are applications where it’s necessary to override the room setpoint of the master (room-unit).
// For instance in situations where room controls are connected to home or building controls or room controls in
// holiday houses which are activated/controlled remotely.
//
// The master can read on Data ID 9 the remote override room setpoint. A value unequal to zero is a valid
// remote override room setpoint. A value of zero means no remote override room setpoint. ID100 defines how
// the master should react while remote room setpoint is active and there is a manual setpoint change and/or a
// program setpoint change.

// #9: Remote override room setpoint
void OpenThermGWClimate::process_Slave_MSG_TROVERRIDE(uint32_t &response) {
    float remote_override_room_setpoint = getFloat(response);
    ESP_LOGD(TAG, "remote_override_room_setpoint: %f", remote_override_room_setpoint);
}

// #100:  Remote override function
void OpenThermGWClimate::process_Slave_MSG_REMOTE_OVERRIDE_FUNCTION(uint32_t &response) {
    uint8_t remote_override_function = getLBUInt8(response);
    ESP_LOGD(TAG, "remote_override_function: %d", remote_override_function);
}


}  // namespace opentherm
}  // namespace esphome

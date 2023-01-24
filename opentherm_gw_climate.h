#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_mode.h"
#include "esphome/components/climate/climate_traits.h"
#include "esphome/components/custom/climate/custom_climate.h"
#include "opentherm.h"

namespace esphome {
namespace opentherm {

class OpenThermGWClimate : public climate::Climate, public Component {
 public:
  OpenThermGWClimate(InternalGPIOPin *m_pin_in, InternalGPIOPin *m_pin_out, InternalGPIOPin *s_pin_in, InternalGPIOPin *s_pin_out);
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;
  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  void processRequest(uint32_t request, OpenThermResponseStatus status);
  void processResponse(uint32_t request, uint32_t &response, OpenThermResponseStatus status);

  void process_Master_MSG_COMMAND(uint32_t &request);
  void process_Master_MSG_DATE(uint32_t &request);
  void process_Master_MSG_DAY_TIME(uint32_t &request);
  void process_Master_MSG_MASTER_VERSION(uint32_t &request);
  void process_Master_MSG_M_CONFIG_M_MEMBERIDCODE(uint32_t &request);
  void process_Master_MSG_OPENTHERM_VERSION_MASTER(uint32_t &request);
  void process_Master_MSG_STATUS(uint32_t &request);
  void process_Master_MSG_TR(uint32_t &request);
  void process_Master_MSG_TRSET(uint32_t &request);
  void process_Master_MSG_TRSETCH2(uint32_t &request);
  void process_Master_MSG_TSET(uint32_t &request);
  void process_Master_MSG_TSETCH2(uint32_t &request);
  void process_Master_MSG_YEAR(uint32_t &request);
  void process_Master_MSG_TDHWSET(uint32_t &request);
  void process_Master_MSG_MAXTSET(uint32_t &request);
  void process_Master_MSG_TSP_INDEX_TSP_VALUE(uint32_t &response);
  void process_Master_MSG_COOLING_CONTROL(uint32_t &request);
  void process_Master_MSG_MAX_REL_MOD_LEVEL_SETTING(uint32_t &request);

  void process_Slave_MSG_ASF_FLAGS_OEM_FAULT_CODE(uint32_t &response);
  void process_Slave_MSG_BURNER_OPERATION_HOURS(uint32_t &response);
  void process_Slave_MSG_BURNER_STARTS(uint32_t &response);
  void process_Slave_MSG_CH_PRESSURE(uint32_t &response);
  void process_Slave_MSG_CH_PUMP_OPERATION_HOURS(uint32_t &response);
  void process_Slave_MSG_CH_PUMP_STARTS(uint32_t &response);
  void process_Slave_MSG_COMMAND(uint32_t &response);
  void process_Slave_MSG_DATE(uint32_t &response);
  void process_Slave_MSG_DAY_TIME(uint32_t &response);
  void process_Slave_MSG_DHW_BURNER_OPERATION_HOURS(uint32_t &response);
  void process_Slave_MSG_DHW_BURNER_STARTS(uint32_t &response);
  void process_Slave_MSG_DHW_FLOW_RATE(uint32_t &response);
  void process_Slave_MSG_DHW_PUMP_VALVE_OPERATION_HOURS(uint32_t &response);
  void process_Slave_MSG_DHW_PUMP_VALVE_STARTS(uint32_t &response);
  void process_Slave_MSG_OEM_DIAGNOSTIC_CODE(uint32_t &response);
  void process_Slave_MSG_OPENTHERM_VERSION_SLAVE(uint32_t &response);
  void process_Slave_MSG_REL_MOD_LEVEL(uint32_t &response);
  void process_Slave_MSG_REMOTE_OVERRIDE_FUNCTION(uint32_t &response);
  void process_Slave_MSG_SLAVE_VERSION(uint32_t &response);
  void process_Slave_MSG_STATUS(uint32_t &response);
  void process_Slave_MSG_S_CONFIG_S_MEMBERIDCODE(uint32_t &response);
  void process_Slave_MSG_TBOILER(uint32_t &response);
  void process_Slave_MSG_TCOLLECTOR(uint32_t &response);
  void process_Slave_MSG_TDHW(uint32_t &response);
  void process_Slave_MSG_TDHW2(uint32_t &response);
  void process_Slave_MSG_TEXHAUST(uint32_t &response);
  void process_Slave_MSG_TFLOWCH2(uint32_t &response);
  void process_Slave_MSG_TOUTSIDE(uint32_t &response);
  void process_Slave_MSG_TRET(uint32_t &response);
  void process_Slave_MSG_TROVERRIDE(uint32_t &response);
  void process_Slave_MSG_TSTORAGE(uint32_t &response);
  void process_Slave_MSG_YEAR(uint32_t &response);
  void process_Slave_MSG_RBP_FLAGS(uint32_t &response);
  void process_Slave_MSG_TDHWSET_UB_LB(uint32_t &response);
  void process_Slave_MSG_MAXTSET_UB_LB(uint32_t &response);
  void process_Slave_MSG_TDHWSET(uint32_t &request);
  void process_Slave_MSG_MAXTSET(uint32_t &request);
  void process_Slave_MSG_TSP(uint32_t &response);
  void process_Slave_MSG_TSP_INDEX_TSP_VALUE(uint32_t &response);
  void process_Slave_MSG_FHB_SIZE(uint32_t &response);
  void process_Slave_MSG_FHB_INDEX_FHB_VALUE(uint32_t &response);
  void process_Slave_MSG_MAX_CAPACITY_MIN_MOD_LEVEL(uint32_t &response);

  OpenThermChannel mOT;
  OpenThermChannel sOT;

public:

  // If a maximum relative modulation level value has been configured, the gateway
  // will send the configured setpoint instead of the one received from the thermostat.
  optional<float> max_relative_modulation_level;

  // If a hot water setpoint value has been configured, the gateway will send
  // the configured setpoint instead of the one received from the thermostat.
  optional<float> dhw_setpoint;

  // If a max central heating setpoint value has been configured, the gateway will send
  // the configured setpoint instead of the one received from the thermostat.
  optional<float> max_ch_water_setpoint;

  binary_sensor::BinarySensor *ch2_active{nullptr};
  binary_sensor::BinarySensor *ch_active{nullptr};
  binary_sensor::BinarySensor *cooling_active{nullptr};
  binary_sensor::BinarySensor *dhw_active{nullptr};
  binary_sensor::BinarySensor *diagnostic_event{nullptr};
  binary_sensor::BinarySensor *fault_indication{nullptr};
  binary_sensor::BinarySensor *flame_on{nullptr};
  sensor::Sensor *boiler_water_temp{nullptr};
  sensor::Sensor *burner_operation_hours{nullptr};
  sensor::Sensor *burner_starts{nullptr};
  sensor::Sensor *ch_pump_operation_hours{nullptr};
  sensor::Sensor *ch_pump_starts{nullptr};
  sensor::Sensor *ch_water_pressure{nullptr};
  sensor::Sensor *dhw2_temperature{nullptr};
  sensor::Sensor *dhw_burner_operation_hours{nullptr};
  sensor::Sensor *dhw_burner_starts{nullptr};
  sensor::Sensor *dhw_flow_rate{nullptr};
  sensor::Sensor *dhw_pump_valve_operation_hours{nullptr};
  sensor::Sensor *dhw_pump_valve_starts{nullptr};
  sensor::Sensor *dhw_temperature{nullptr};
  sensor::Sensor *exhaust_temperature{nullptr};
  sensor::Sensor *flow_temperature_ch2{nullptr};
  sensor::Sensor *outside_air_temperature{nullptr};
  sensor::Sensor *relative_modulation_level{nullptr};
  sensor::Sensor *return_water_temperature{nullptr};
  sensor::Sensor *solar_collector_temperature{nullptr};
  sensor::Sensor *solar_storage_temperature{nullptr};
};

}  // namespace opentherm
}  // namespace esphome

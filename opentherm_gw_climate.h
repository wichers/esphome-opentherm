#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_mode.h"
#include "esphome/components/climate/climate_traits.h"
#include "esphome/components/custom/climate/custom_climate.h" 
#include "opentherm.h"

namespace esphome {
namespace opentherm {

struct OpenThermGWClimateTargetTempConfig {
 public:
  OpenThermGWClimateTargetTempConfig();
  OpenThermGWClimateTargetTempConfig(float default_temperature_low, float default_temperature_high);

  float default_temperature_low{NAN};
  float default_temperature_high{NAN};
};

class OpenThermGWClimate : public climate::Climate, public Component {
 public:
  OpenThermGWClimate(GPIOPin *m_pin_in, GPIOPin *m_pin_out, GPIOPin *s_pin_in, GPIOPin *s_pin_out);
  void setup() override;
  void dump_config() override;
  void loop() override;

  void set_sensor(sensor::Sensor *sensor);
  Trigger<> *get_idle_trigger() const;
  Trigger<> *get_heat_trigger() const;
  void set_supports_heat(bool supports_heat);
  void set_normal_config(const OpenThermGWClimateTargetTempConfig &normal_config);
  void set_away_config(const OpenThermGWClimateTargetTempConfig &away_config);

 protected:
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;
  /// Change the away setting, will reset target temperatures to defaults.
  void change_away_(bool away);
  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// Re-compute the state of this climate controller.
  void compute_state_();

  /// Switch the climate device to the given climate mode.
  void switch_to_action_(climate::ClimateAction action);

  /// The sensor used for getting the current temperature
  sensor::Sensor *sensor_{nullptr};
  /** The trigger to call when the controller should switch to idle mode.
   *
   * In idle mode, the controller is assumed to have heating disabled.
   */
  Trigger<> *idle_trigger_;
  /** The trigger to call when the controller should switch to heating mode.
   *
   * A null value for this attribute means that the controller has no heating action.
   */
  Trigger<> *heat_trigger_{nullptr};
  bool supports_heat_{false};
  /** A reference to the trigger that was previously active.
   *
   * This is so that the previous trigger can be stopped before enabling a new one.
   */
  Trigger<> *prev_trigger_{nullptr};

  OpenThermGWClimateTargetTempConfig normal_config_{};
  bool supports_away_{false};
  OpenThermGWClimateTargetTempConfig away_config_{};

  OpenTherm mOT;
  OpenTherm sOT;

  void processRequest(uint32_t request, OpenThermResponseStatus status);
  void processResponse(const uint32_t response);
    
  void process_Master_MSG_COMMAND(const uint32_t request);
  void process_Master_MSG_DATE(const uint32_t request);
  void process_Master_MSG_DAY_TIME(const uint32_t request);
  void process_Master_MSG_MASTER_VERSION(const uint32_t request);
  void process_Master_MSG_M_CONFIG_M_MEMBERIDCODE(const uint32_t request);
  void process_Master_MSG_OPENTHERM_VERSION_MASTER(const uint32_t request);
  void process_Master_MSG_STATUS(const uint32_t request);
  void process_Master_MSG_TR(const uint32_t request);
  void process_Master_MSG_TRSET(const uint32_t request);
  void process_Master_MSG_TRSETCH2(const uint32_t request);
  void process_Master_MSG_TSET(const uint32_t request);
  void process_Master_MSG_TSETCH2(const uint32_t request);
  void process_Master_MSG_YEAR(const uint32_t request);
  void process_Slave_MSG_ASF_FLAGS_OEM_FAULT_CODE(const uint32_t response);
  void process_Slave_MSG_BURNER_OPERATION_HOURS(const uint32_t response);
  void process_Slave_MSG_BURNER_STARTS(const uint32_t response);
  void process_Slave_MSG_CH_PRESSURE(const uint32_t response);
  void process_Slave_MSG_CH_PUMP_OPERATION_HOURS(const uint32_t response);
  void process_Slave_MSG_CH_PUMP_STARTS(const uint32_t response);
  void process_Slave_MSG_COMMAND(const uint32_t response);
  void process_Slave_MSG_DATE(const uint32_t response);
  void process_Slave_MSG_DAY_TIME(const uint32_t response);
  void process_Slave_MSG_DHW_BURNER_OPERATION_HOURS(const uint32_t response);
  void process_Slave_MSG_DHW_BURNER_STARTS(const uint32_t response);
  void process_Slave_MSG_DHW_FLOW_RATE(const uint32_t response);
  void process_Slave_MSG_DHW_PUMP_VALVE_OPERATION_HOURS(const uint32_t response);
  void process_Slave_MSG_DHW_PUMP_VALVE_STARTS(const uint32_t response);
  void process_Slave_MSG_OEM_DIAGNOSTIC_CODE(const uint32_t response);
  void process_Slave_MSG_OPENTHERM_VERSION_SLAVE(const uint32_t response);
  void process_Slave_MSG_REL_MOD_LEVEL(const uint32_t response);
  void process_Slave_MSG_REMOTE_OVERRIDE_FUNCTION(const uint32_t response);
  void process_Slave_MSG_SLAVE_VERSION(const uint32_t response);
  void process_Slave_MSG_STATUS(const uint32_t response);
  void process_Slave_MSG_S_CONFIG_S_MEMBERIDCODE(const uint32_t response);
  void process_Slave_MSG_TBOILER(const uint32_t response);
  void process_Slave_MSG_TCOLLECTOR(const uint32_t response);
  void process_Slave_MSG_TDHW(const uint32_t response);
  void process_Slave_MSG_TDHW2(const uint32_t response);
  void process_Slave_MSG_TEXHAUST(const uint32_t response);
  void process_Slave_MSG_TFLOWCH2(const uint32_t response);
  void process_Slave_MSG_TOUTSIDE(const uint32_t response);
  void process_Slave_MSG_TRET(const uint32_t response);
  void process_Slave_MSG_TROVERRIDE(const uint32_t response);
  void process_Slave_MSG_TSTORAGE(const uint32_t response);
  void process_Slave_MSG_YEAR(const uint32_t response);

};

}  // namespace opentherm
}  // namespace esphome

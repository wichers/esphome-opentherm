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

class OpenThermGWClimate : public climate::Climate, public Component {
 public:
  OpenThermGWClimate(GPIOPin *m_pin_in, GPIOPin *m_pin_out, GPIOPin *s_pin_in, GPIOPin *s_pin_out);
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;
  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

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
  void process_Master_MSG_TDHWSET(const uint32_t request);
  void process_Master_MSG_MAXTSET(const uint32_t request);
  void process_Master_MSG_TSP_INDEX_TSP_VALUE(const uint32_t response);
  void process_Master_MSG_COOLING_CONTROL(const uint32_t request);
  void process_Master_MSG_MAX_REL_MOD_LEVEL_SETTING(const uint32_t request);

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
  void process_Slave_MSG_RBP_FLAGS(const uint32_t response);
  void process_Slave_MSG_TDHWSET_UB_LB(const uint32_t response);
  void process_Slave_MSG_MAXTSET_UB_LB(const uint32_t response);
  void process_Slave_MSG_TDHWSET(const uint32_t request);
  void process_Slave_MSG_MAXTSET(const uint32_t request);
  void process_Slave_MSG_TSP(const uint32_t response);
  void process_Slave_MSG_TSP_INDEX_TSP_VALUE(const uint32_t response);
  void process_Slave_MSG_FHB_SIZE(const uint32_t response);
  void process_Slave_MSG_FHB_INDEX_FHB_VALUE(const uint32_t response);
  void process_Slave_MSG_MAX_CAPACITY_MIN_MOD_LEVEL(const uint32_t response);

  OpenThermChannel mOT;
  OpenThermChannel sOT;
};

}  // namespace opentherm
}  // namespace esphome

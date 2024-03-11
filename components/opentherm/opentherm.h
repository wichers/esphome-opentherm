#pragma once 
/*
OpenTherm.h - OpenTherm Library for the ESP8266/Arduino platform
https://github.com/ihormelnyk/OpenTherm
http://ihormelnyk.com/pages/OpenTherm
Licensed under MIT license
Copyright 2018, Ihor Melnyk

Frame Structure:
P MGS-TYPE SPARE DATA-ID  DATA-VALUE
0 000      0000  00000000 00000000 00000000
*/

#include <esphome/core/hal.h>
#include <esphome/core/gpio.h>
#include <functional>

namespace esphome {
namespace opentherm { 

enum OpenThermResponseStatus {
NONE,
SUCCESS,
INVALID,
TIMEOUT
};


enum OpenThermMessageType {
/*  Master to Slave */
READ_DATA       = 0,
READ            = READ_DATA, // for backwared compatibility
WRITE_DATA      = 1,
WRITE           = WRITE_DATA, // for backwared compatibility
INVALID_DATA    = 2,
RESERVED        = 3,
/* Slave to Master */
READ_ACK        = 4,
WRITE_ACK       = 5,
DATA_INVALID    = 6,
UNKNOWN_DATA_ID = 7
};

typedef OpenThermMessageType OpenThermRequestType; // for backwared compatibility

enum OpenThermMessageID {
// Master and Slave Status flags.
MSG_STATUS = 0,
// Control setpoint ie CH water temperature setpoint (°C)
MSG_TSET = 1,
// Master Configuration Flags / Master MemberID Code
MSG_M_CONFIG_M_MEMBERIDCODE = 2,
// Slave Configuration Flags / Slave MemberID Code
MSG_S_CONFIG_S_MEMBERIDCODE = 3,
// Remote Command
MSG_COMMAND = 4,
// Application-specific fault flags and OEM fault code
MSG_ASF_FLAGS_OEM_FAULT_CODE = 5,
// Remote boiler parameter transfer-enable & read/write flags
MSG_RBP_FLAGS = 6,
// Cooling control signal (%)
MSG_COOLING_CONTROL = 7,
// Control setpoint for 2nd CH circuit (°C)
MSG_TSETCH2 = 8,
// Remote override room setpoint
MSG_TROVERRIDE = 9,
// Number of Transparent-Slave-Parameters supported by slave
MSG_TSP = 10,
// Index number / Value of referred-to transparent slave parameter.
MSG_TSP_INDEX_TSP_VALUE = 11,
// Size of Fault-History-Buffer supported by slave
MSG_FHB_SIZE = 12,
// Index number / Value of referred-to fault-history buffer entry.
MSG_FHB_INDEX_FHB_VALUE = 13,
// Maximum relative modulation level setting (%)
MSG_MAX_REL_MOD_LEVEL_SETTING = 14,
// Maximum boiler capacity (kW) / Minimum boiler modulation level(%)
MSG_MAX_CAPACITY_MIN_MOD_LEVEL = 15,
// Room Setpoint (°C)
MSG_TRSET = 16,
// Relative Modulation Level (%)
MSG_REL_MOD_LEVEL = 17,
// Water pressure in CH circuit (bar)
MSG_CH_PRESSURE = 18,
// Water flow rate in DHW circuit. (litres/minute)
MSG_DHW_FLOW_RATE = 19,
// Day of Week and Time of Day
MSG_DAY_TIME = 20,
// Calendar date
MSG_DATE = 21,
// Calendar year
MSG_YEAR = 22,
// Room Setpoint for 2nd CH circuit (°C)
MSG_TRSETCH2 = 23,
// Room temperature (°C)
MSG_TR = 24,
// Boiler flow water temperature (°C)
MSG_TBOILER = 25,
// DHW temperature (°C)
MSG_TDHW = 26,
// Outside temperature (°C)
MSG_TOUTSIDE = 27,
// Return water temperature (°C)
MSG_TRET = 28,
// Solar storage temperature (°C)
MSG_TSTORAGE = 29,
// Solar collector temperature (°C)
MSG_TCOLLECTOR = 30,
// Flow water temperature CH2 circuit (°C)
MSG_TFLOWCH2 = 31,
// Domestic hot water temperature 2 (°C)
MSG_TDHW2 = 32,
// Boiler exhaust temperature (°C)
MSG_TEXHAUST = 33,
// DHW setpoint upper & lower bounds for adjustment (°C)
MSG_TDHWSET_UB_LB = 48,
// Max CH water setpoint upper & lower bounds for adjustment (°C)
MSG_MAXTSET_UB_LB = 49,
// OTC heat curve ratio upper & lower bounds for adjustment
MSG_HCRATIO_UB_LB = 50,
// DHW setpoint (°C) (Remote parameter 1)
MSG_TDHWSET = 56,
// Max CH water setpoint (°C) (Remote parameters 2)
MSG_MAXTSET = 57,
// OTC heat curve ratio (°C) (Remote parameter 3)
MSG_HCRATIO = 58,
// Function of manual and program changes in master and remote room setpoint.
MSG_REMOTE_OVERRIDE_FUNCTION = 100,
// OEM-specific diagnostic/service code
MSG_OEM_DIAGNOSTIC_CODE = 115,
// Number of starts burner
MSG_BURNER_STARTS = 116,
// Number of starts CH pump
MSG_CH_PUMP_STARTS = 117,
// Number of starts DHW pump/valve
MSG_DHW_PUMP_VALVE_STARTS = 118,
// Number of starts burner during DHW mode
MSG_DHW_BURNER_STARTS = 119,
// Number of hours that burner is in operation (i.e. flame on)
MSG_BURNER_OPERATION_HOURS = 120,
// Number of hours that CH pump has been running
MSG_CH_PUMP_OPERATION_HOURS = 121,
// Number of hours that DHW pump has been running or DHW valve has been opened
MSG_DHW_PUMP_VALVE_OPERATION_HOURS = 122,
// Number of hours that burner is in operation during DHW mode
MSG_DHW_BURNER_OPERATION_HOURS = 123,
// The implemented version of the OpenTherm Protocol Specification in the master.
MSG_OPENTHERM_VERSION_MASTER = 124,
// The implemented version of the OpenTherm Protocol Specification in the slave.
MSG_OPENTHERM_VERSION_SLAVE = 125,
// Master product version number and type
MSG_MASTER_VERSION = 126,
// Slave product version number and type
MSG_SLAVE_VERSION = 127
};

enum OpenThermStatus {
NOT_INITIALIZED,
READY,
DELAY,
REQUEST_SENDING,
RESPONSE_WAITING,
RESPONSE_START_BIT,
RESPONSE_RECEIVING,
RESPONSE_READY,
RESPONSE_INVALID
};

struct OpenThermStore {
  OpenThermStore(bool slave = false) 
  : isSlave(slave)
  {}
  static void gpio_intr(OpenThermStore *arg);

  ISRInternalGPIOPin pin_in;
  volatile uint32_t response{0};
  volatile uint32_t responseTimestamp{0};
  volatile uint8_t responseBitIndex{0};
  volatile OpenThermStatus status{OpenThermStatus::NOT_INITIALIZED};
  const bool isSlave;
};

class OpenThermChannel
{
public:
  OpenThermChannel(bool isSlave = false);
  ~OpenThermChannel();

  void set_pin_in(InternalGPIOPin *pin_in) {this->pin_in_ = pin_in;}
  void set_pin_out(InternalGPIOPin *pin_out) {this->pin_out_ = pin_out;}

  void setup(std::function<void(uint32_t, OpenThermResponseStatus)> callback);
  void loop();
  uint32_t sendRequest(uint32_t request);
  bool sendResponse(uint32_t request);
  OpenThermResponseStatus getLastResponseStatus();

protected:
  bool sendRequestAync(uint32_t request);
  bool isReady();
  void setActiveState();
  void setIdleState();
  void activateBoiler();
  void sendBit(bool high);

  std::function<void(uint32_t, OpenThermResponseStatus)> process_response_callback;
  InternalGPIOPin *pin_in_;
  InternalGPIOPin *pin_out_;
  const bool isSlave;
  OpenThermResponseStatus responseStatus;
  OpenThermStore store_;
};

const char *statusToString(OpenThermResponseStatus status);
uint32_t buildRequest(OpenThermMessageType type, OpenThermMessageID id, uint16_t data);
uint32_t buildResponse(OpenThermMessageType type, OpenThermMessageID id, uint16_t data);
bool parity(uint32_t frame);
OpenThermMessageType getMessageType(uint32_t message);
OpenThermMessageID getDataID(uint32_t frame);
const char *messageTypeToString(OpenThermMessageType message_type);
bool isValidRequest(uint32_t request);
bool isValidResponse(uint32_t response);
uint32_t modifyMsgData(uint32_t msg, uint16_t data);
uint8_t getUBUInt8(const uint32_t response);
uint8_t getLBUInt8(const uint32_t response);
int8_t getUBInt8(const uint32_t response);
int8_t getLBInt8(const uint32_t response);
uint16_t getUInt16(const uint32_t response);
int16_t getInt16(const uint32_t response);
float getFloat(const uint32_t response);  
uint16_t temperatureToData(float temperature);

}  // namespace opentherm
}  // namespace esphome 

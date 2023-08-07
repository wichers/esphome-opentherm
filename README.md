# OpenTherm
Port of OpenTherm protocol to ESPHome.io firmware.

## Hardware:
https://github.com/jpraus/arduino-opentherm
http://ihormelnyk.com/opentherm_adapter
https://diyless.com/product/esp8266-opentherm-gateway

## Software:
Publishes multiple sensors to Home Assistant, and a service to override the ch setpoint in case the boiler misses MSGid9 in the opentherm implementation.

### CH override service:
To use the service, send a value > 0 to the gateway using the "ch temperature override setpoint"-service. It will auto enable the CH function and set the target temperature to what you've sent. setting it to 0 disables the CH function.

## Notes:
The ESP32 version of the YAML compiles, but is untested on real hardware.

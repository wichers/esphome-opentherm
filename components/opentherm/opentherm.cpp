/* OpenTherm.cpp - OpenTherm Communication Library For Arduino, ESP8266
Copyright 2018, Ihor Melnyk */

#include "opentherm.h"
#include <esphome/core/helpers.h>

namespace esphome {
namespace opentherm {

OpenThermChannel::OpenThermChannel(bool slave):
  isSlave(slave),
  store_(slave)
{
}

OpenThermChannel::~OpenThermChannel() {
  this->pin_in_->detach_interrupt();
}

void OpenThermChannel::setup(std::function<void(uint32_t, OpenThermResponseStatus)> callback)
{
  this->pin_in_->setup();
  this->store_.pin_in = this->pin_in_->to_isr();

  this->pin_out_->setup();

  this->pin_in_->attach_interrupt(OpenThermStore::gpio_intr, &this->store_, gpio::INTERRUPT_ANY_EDGE);

  activateBoiler();
  this->store_.status = OpenThermStatus::READY;
  this->process_response_callback = callback;
}

void OpenThermChannel::loop()
{
  OpenThermStatus st;
  uint32_t ts;
  {
    InterruptLock lock;
    st = this->store_.status;
    ts = this->store_.responseTimestamp;
  }

  if (st == OpenThermStatus::READY) return;
  uint32_t newTs = micros();
  if (st != OpenThermStatus::NOT_INITIALIZED && (newTs - ts) > 1000000) {
    this->store_.status = OpenThermStatus::READY;
    responseStatus = OpenThermResponseStatus::TIMEOUT;
    if (process_response_callback) {
      process_response_callback(this->store_.response, responseStatus);
    }
  }
  else if (st == OpenThermStatus::RESPONSE_INVALID) {
    this->store_.status = OpenThermStatus::DELAY;
    responseStatus = OpenThermResponseStatus::INVSTART;
    if (process_response_callback) {
      process_response_callback(this->store_.response, responseStatus);
    }
  }
  else if (st == OpenThermStatus::RESPONSE_READY) {
    this->store_.status = OpenThermStatus::DELAY;
    if (parity(this->store_.response))
      responseStatus = OpenThermResponseStatus::INVPARITY;
    else if (isSlave)
      responseStatus = isValidResponse(this->store_.response) ? OpenThermResponseStatus::SUCCESS : OpenThermResponseStatus::INVMSGTYPE;
    else
      responseStatus = isValidRequest(this->store_.response) ? OpenThermResponseStatus::SUCCESS : OpenThermResponseStatus::INVMSGTYPE;
    if (process_response_callback) {
      process_response_callback(this->store_.response, responseStatus);
    }
  }
  else if (st == OpenThermStatus::DELAY) {
    if ((newTs - ts) > 100000) {
      this->store_.status = OpenThermStatus::READY;
    }
  }
}

bool OpenThermChannel::isReady()
{
  return this->store_.status == OpenThermStatus::READY;
}

void OpenThermChannel::setActiveState() {
  this->pin_out_->digital_write(false);
}

void OpenThermChannel::setIdleState() {
  this->pin_out_->digital_write(true);
}

void OpenThermChannel::activateBoiler() {
  setIdleState();
  delay(1000);
}

void OpenThermChannel::sendBit(bool high) {
  if (high) setActiveState(); else setIdleState();
  delayMicroseconds(500);
  if (high) setIdleState(); else setActiveState();
  delayMicroseconds(500);
}

bool OpenThermChannel::sendRequestAync(uint32_t request)
{
  bool ready;
  {
    InterruptLock lock;
    ready = isReady();
  }

  if (!ready)
    return false;

  this->store_.status = OpenThermStatus::REQUEST_SENDING;
  this->store_.response = 0;
  responseStatus = OpenThermResponseStatus::NONE;

  sendBit(true); //start bit
  for (int i = 31; i >= 0; i--) {
    sendBit((request & (1 << i)) != 0);
  }
  sendBit(true); //stop bit
  setIdleState();

  this->store_.status = OpenThermStatus::RESPONSE_WAITING;
  this->store_.responseTimestamp = micros();
  return true;
}

uint32_t OpenThermChannel::sendRequest(uint32_t request)
{
  if (!sendRequestAync(request)) return 0;
  while (!isReady()) {
    loop();
    yield();
  }
  return this->store_.response;
}

bool OpenThermChannel::sendResponse(uint32_t request)
{
  this->store_.status = OpenThermStatus::REQUEST_SENDING;
  this->store_.response = 0;
  responseStatus = OpenThermResponseStatus::NONE;

  sendBit(true); //start bit
  for (int i = 31; i >= 0; i--) {
    sendBit((request & (1 << i)) != 0);
  }
  sendBit(true); //stop bit
  setIdleState();
  this->store_.status = OpenThermStatus::READY;
  return true;
}

OpenThermResponseStatus OpenThermChannel::getLastResponseStatus()
{
  return responseStatus;
}

void IRAM_ATTR OpenThermStore::gpio_intr(OpenThermStore *arg)
{
  if (arg->status == OpenThermStatus::READY)
  {
    if (!arg->isSlave && arg->pin_in.digital_read()) {
       arg->status = OpenThermStatus::RESPONSE_WAITING;
    }
    else {
      return;
    }
  }

  uint32_t newTs = micros();
  if (arg->status == OpenThermStatus::RESPONSE_WAITING) {
    if (arg->pin_in.digital_read()) {
      arg->status = OpenThermStatus::RESPONSE_START_BIT;
      arg->responseTimestamp = newTs;
    }
    else {
      arg->status = OpenThermStatus::RESPONSE_INVALID;
      arg->responseTimestamp = newTs;
    }
  }
  else if (arg->status == OpenThermStatus::RESPONSE_START_BIT) {
    if ((newTs - arg->responseTimestamp < 750) && !arg->pin_in.digital_read()) {
      arg->status = OpenThermStatus::RESPONSE_RECEIVING;
      arg->responseTimestamp = newTs;
      arg->responseBitIndex = 0;
    }
    else {
      arg->status = OpenThermStatus::RESPONSE_INVALID;
      arg->responseTimestamp = newTs;
    }
  }
  else if (arg->status == OpenThermStatus::RESPONSE_RECEIVING) {
    if ((newTs - arg->responseTimestamp) > 750) {
      if (arg->responseBitIndex < 32) {
        arg->response = (arg->response << 1) | !arg->pin_in.digital_read();
        arg->responseTimestamp = newTs;
        arg->responseBitIndex++;
      }
      else { //stop bit
        arg->status = OpenThermStatus::RESPONSE_READY;
        arg->responseTimestamp = newTs;
      }
    }
  }
}

bool parity(uint32_t frame) //odd parity
{
  uint8_t p = 0;
  while (frame > 0)
  {
    if (frame & 1) p++;
    frame = frame >> 1;
  }
  return (p & 1);
}

OpenThermMessageType getMessageType(uint32_t message)
{
  OpenThermMessageType msg_type = static_cast<OpenThermMessageType>((message >> 28) & 7);
  return msg_type;
}

OpenThermMessageID getDataID(uint32_t frame)
{
  return (OpenThermMessageID)((frame >> 16) & 0xFF);
}

uint32_t buildRequest(OpenThermMessageType type, OpenThermMessageID id, uint16_t data)
{
  uint32_t request = data;
  if (type == OpenThermMessageType::WRITE_DATA) {
    request |= 1ul << 28;
  }
  request |= ((uint32_t)id) << 16;
  if (parity(request)) request |= (1ul << 31);
  return request;
}

uint32_t buildResponse(OpenThermMessageType type, OpenThermMessageID id, uint16_t data)
{
  uint32_t response = data;
  response |= type << 28;
  response |= ((uint32_t)id) << 16;
  if (parity(response)) response |= (1ul << 31);
  return response;
}

uint32_t modifyMsgData(uint32_t msg, uint16_t data)
{
  msg = (msg & 0x7fff0000) | data;
  if (parity(msg)) msg |= (1ul << 31);
  return msg;
}

bool isValidResponse(uint32_t response)
{
  uint8_t msgType = (response << 1) >> 29;
  return msgType == READ_ACK || msgType == WRITE_ACK || msgType == DATA_INVALID || msgType == UNKNOWN_DATA_ID;
}

bool isValidRequest(uint32_t request)
{
  uint8_t msgType = (request << 1) >> 29;
  return msgType == READ_DATA || msgType == WRITE_DATA || msgType == INVALID_DATA;
}

const char *statusToString(OpenThermResponseStatus status)
{
  switch (status) {
    case NONE:  return "NONE";
    case SUCCESS: return "SUCCESS";
    case TIMEOUT: return "TIMEOUT";
    case INVSTART: return "INVSTART";
    case INVPARITY: return "INVPARITY";
    case INVMSGTYPE: return "INVMSGTYPE";
  }
  return "UNKNOWN";
}

const char *messageTypeToString(OpenThermMessageType message_type)
{
  switch (message_type) {
    case READ_DATA:    return "READ_DATA";
    case WRITE_DATA:    return "WRITE_DATA";
    case INVALID_DATA:  return "INVALID_DATA";
    case RESERVED:    return "RESERVED";
    case READ_ACK:    return "READ_ACK";
    case WRITE_ACK:    return "WRITE_ACK";
    case DATA_INVALID:  return "DATA_INVALID";
    case UNKNOWN_DATA_ID: return "UNKNOWN_DATA_ID";
    default:        return "UNKNOWN";
  }
}

uint8_t getUBUInt8(const uint32_t response) {
  return (response >> 8) & 0xff;
}

uint8_t getLBUInt8(const uint32_t response) {
  return response & 0xff;
}

int8_t getUBInt8(const uint32_t response) {
  return (int8_t) ((response >> 8) & 0xff);
}

int8_t getLBInt8(const uint32_t response) {
  return (int8_t) (response & 0xff);
}

uint16_t getUInt16(const uint32_t response) {
  return response & 0xffff;
}

int16_t getInt16(const uint32_t response) {
  return (int16_t) (response & 0xffff);
}

float getFloat(const uint32_t response) {
  const uint16_t u88 = getUInt16(response);
  const float f = (u88 & 0x8000) ? -(0x10000L - u88) / 256.0f : u88 / 256.0f;
  return f;
}

uint16_t temperatureToData(float temperature) {
  if (temperature < 0) temperature = 0;
  if (temperature > 100) temperature = 100;
  uint16_t data = (uint16_t)(temperature * 256);
  return data;
}

}  // namespace opentherm
}  // namespace esphome

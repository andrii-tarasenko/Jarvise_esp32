#ifndef INVERTER_READER_H
#define INVERTER_READER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ApiClient.h"
#include <HardwareSerial.h>

struct InverterData {
  bool isValid;
  float grid_voltage;
  float grid_freq;
  float output_voltage;
  float output_freq;
  int output_va;
  int output_power;
  int output_load_percent;
  int bus_voltage;
  float battery_voltage;
  int battery_charging_current;
  int battery_capacity;
  int inverter_heatsink_temp;
  float pv_input_current_for_battery;
  float pv_input_voltage;
  float battery_voltage_from_scc;
  int battery_discharge_current;
  float pv_input_power_approx;
};

class InverterReader {
public:
  InverterReader(int rxPin, int txPin);
  void begin();
  void process();
  InverterData getData();

private:
  HardwareSerial serialPort;
  int rxPin;
  int txPin;

  InverterData lastData;

  unsigned long lastCommandTime;
  unsigned long lastReceiveTime;
  
  // State machine indicators
  bool waitingForResponse;
  String buffer;

  // Helpers
  void sendCommand(String command);
  unsigned int calcCRC(const char *cmd);
  void parseData(String payload);
};

#endif // INVERTER_READER_H

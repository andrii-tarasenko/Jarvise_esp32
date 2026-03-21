#ifndef INVERTER_READER_H
#define INVERTER_READER_H

#include <Arduino.h>
#include <ArduinoJson.h>

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

#include <ModbusMaster.h>

class InverterReader {
public:
  InverterReader(int rxPin, int txPin);
  void begin();
  void process();
  InverterData getData();
  
  void startScanner();
  void testLoopback();
  bool isScannerActive() { return scannerActive; }

private:
  int rxPin;
  int txPin;
  ModbusMaster node;

  InverterData lastData;

  unsigned long lastCommandTime;
  unsigned long lastReceiveTime;
  unsigned long lastScannerStepTime;
  
  bool waitingForResponse;
  bool scannerActive;
  bool isModbusMode;
  
  int currentBaudIndex;
  int currentSlaveIndex;
  
  const uint32_t baudRates[4] = {2400, 4800, 9600, 19200};
  const uint8_t slaveIds[2] = {0, 1};

  void pollRegisters();
  void pollQpigs();
  void processScanner();
  uint16_t calculateCRC(const char *pin, uint8_t len);
};

#endif // INVERTER_READER_H

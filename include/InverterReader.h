#ifndef IN_READER_H
#define IN_READER_H

#include <Arduino.h>
#include <stdint.h>
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

  // Конструктор для ініціалізації нулями
  InverterData() {
    isValid = false;
    grid_voltage = 0; grid_freq = 0; output_voltage = 0; output_freq = 0;
    output_va = 0; output_power = 0; output_load_percent = 0; bus_voltage = 0;
    battery_voltage = 0; battery_charging_current = 0; battery_capacity = 0;
    inverter_heatsink_temp = 0; pv_input_current_for_battery = 0;
    pv_input_voltage = 0; battery_voltage_from_scc = 0;
    battery_discharge_current = 0; pv_input_power_approx = 0;
  }
};

class InverterReader {
public:
  InverterReader(int rx, int tx);
  void begin(uint32_t baud, bool invert); // Ініціалізація порту
  void process();                 // Основний цикл опитування
  InverterData getData();         // Отримати останні дані

private:
  int rxPin, txPin;
  unsigned long lastCommandTime;
  unsigned long lastScannerStep;
  int scannerIdx; // 0=9600N, 1=9600I, 2=2400N, 3=2400I
  bool scannerActive;
  
  InverterData lastData;

  void pollQpigs();               // Відправка запиту QPIGS
  void processScanner();
  uint16_t calculateCRC(const char *pin, uint8_t len); // Розрахунок контрольної суми
};

#endif // IN_READER_H

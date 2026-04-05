#include "GridSensor.h"
#include <HardwareSerial.h>
#include <math.h>

GridSensor::GridSensor(int rxPin, int txPin)
    : pzem(Serial2, rxPin, txPin) {
  last_power = -1.0;
  last_voltage = -1.0;
}

GridData GridSensor::readData() {
  GridData data;
  data.isValid = false;

  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power(); 
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float power_f = pzem.pf();

  if (isnan(voltage) || isnan(power)) {
    Serial.println("❌ Error reading PZEM (No AC power or disconnected)");
    return data;
  }

    data.isValid = true;
  data.voltage = voltage;
  data.current = current;
  data.power = power;
  data.energy = energy;
  data.frequency = frequency;
  data.power_f = power_f;

  return data;
}

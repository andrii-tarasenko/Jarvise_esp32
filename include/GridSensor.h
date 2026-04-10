#ifndef GRID_SENSOR_H
#define GRID_SENSOR_H

#include <ArduinoJson.h>
#include <PZEM004Tv30.h>

struct GridData {
  bool isValid = false;
  float voltage = 0.0f;
  float current = 0.0f;
  float power = 0.0f;
  float energy = 0.0f;
  float frequency = 0.0f;
  float power_f = 0.0f;
};

struct SensorData {
  float voltage;
  float current;
  float power;
  float energy;
  float frequency;
  float power_f;
};

class GridSensor {
    private:
    PZEM004Tv30 pzem;

    float last_power;
    float last_voltage;

    public:
    GridSensor(int rxPin, int txPin);
    GridData readData();
};

#endif

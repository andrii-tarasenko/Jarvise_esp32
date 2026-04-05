#ifndef GRID_SENSOR_H
#define GRID_SENSOR_H

#include <ArduinoJson.h>
#include <PZEM004Tv30.h>

struct GridData {
  bool isValid;
  float voltage;
  float current;
  float power;
  float energy;
  float frequency;
  float power_f;
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
}

#endif

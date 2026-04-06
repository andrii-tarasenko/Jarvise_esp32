#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>

struct ClimateData {
  float temperature;
  float humidity;
  float pressure;
};

class ClimateSensor {
private:
  Adafruit_BME280 bme;
  bool isInitialized;

public:
  ClimateSensor();
  void begin();
  ClimateData readData();
  bool isReady() const { return isInitialized; }
};

#endif

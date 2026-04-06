#include "ClimateSensor.h"
#include "Env.h"
#include <Wire.h>

ClimateSensor::ClimateSensor() : isInitialized(false) {}

void ClimateSensor::begin() {
  pinMode(BME280_SDA_PIN, INPUT_PULLUP);
  pinMode(BME280_SCL_PIN, OUTPUT);
  for (int i = 0; i < 9; i++) {
    digitalWrite(BME280_SCL_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(BME280_SCL_PIN, HIGH);
    delayMicroseconds(5);
  }

  pinMode(BME280_SDA_PIN, OUTPUT);
  digitalWrite(BME280_SDA_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(BME280_SCL_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(BME280_SDA_PIN, HIGH);
  delayMicroseconds(5);

  Wire.end();
  Wire.begin(BME280_SDA_PIN, BME280_SCL_PIN);

  unsigned status = bme.begin(I2C_BME280_ADDRESS, &Wire);
  if (!status) {
    Serial.println("❌ Could not find a valid BME280 sensor, check wiring!");
    isInitialized = false;
  } else {
    Serial.println("✅ BME280 sensor connected!");
    isInitialized = true;
  }
}

ClimateData ClimateSensor::readData() {
  if (!isInitialized) {
    Serial.println("🔄 BME280 is offline, attempting to reconnect...");
    begin();
  }

  ClimateData data = {0.0, 0.0, 0.0};
  if (isInitialized) {
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity();
    data.pressure = bme.readPressure() / 100.0F;

    if (data.temperature >= 100.0 || data.temperature <= -40.0 || (data.humidity == 100.0 && data.temperature > 50.0)) {
      Serial.print("❌ BME280 read Error! Invalid data detected: Temp=");
      Serial.print(data.temperature);
      Serial.println("C. Marking sensor as offline.");

      data.temperature = 0.0;
      data.humidity = 0.0;
      data.pressure = 0.0;
      isInitialized = false;
    }
  }

  return data;
}

#ifndef ENV_H
#define ENV_H

#include <Arduino.h>

extern const char *WIFI_SSID;
extern const char *WIFI_PASS;

extern const char *SERVER_HOST;
extern const uint16_t SERVER_PORT;
extern const char *VIRTUAL_HOST;
extern const char *API_GRID_ROUTE;
extern const char *API_ROOM_ROUTE;
extern const char *API_LOGS_ROUTE;


extern const unsigned long READ_INTERVAL;
extern const unsigned long END_INTERVAL;

// PZEM-004T Pins
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17

//inverter pins
#define INVERTER_RX_PIN 18
#define INVERTER_TX_PIN 19

// temperature sensor
#define BME280_SDA_PIN 21
#define BME280_SCL_PIN 22
extern const uint8_t I2C_BME280_ADDRESS;

#endif

#ifndef ENV_H
#define ENV_H

#include <Arduino.h>

extern const char* const WIFI_SSID;
extern const char* const WIFI_PASS;

extern const char* const SERVER_HOST;
extern const uint16_t SERVER_PORT;
extern const char* const VIRTUAL_HOST;
extern const char* const API_GRID_ROUTE;
extern const char* const API_ROOM_ROUTE;
extern const char* const API_LOGS_ROUTE;

//requests
extern const char* const RELAY_REQUEST;
extern const char* const CLIMATE_REQUEST;

extern const unsigned long READ_INTERVAL;
extern const unsigned long SEND_INTERVAL;
extern const unsigned long END_INTERVAL;

// Relay
#define NUM_RELAYS 4
extern const int RELAY_PIN_ONE;
extern const int RELAY_PIN_TWO;
extern const int RELAY_PIN_THREE;
extern const int RELAY_PIN_FOUR;

extern const int RELAY_PINS[NUM_RELAYS];

//Motion Pins
#define MOTION_PIN 27

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

#ifndef ENV_H
#define ENV_H

#include <Arduino.h>

//WI-FI
extern const char *WIFI_SSID;
extern const char *WIFI_PASS;

// API Server
extern const char *SERVER_HOST;
extern const uint16_t SERVER_PORT;
extern const char *VIRTUAL_HOST;
extern const char *API_GRID_ROUTE;
extern const char *API_ROOM_ROUTE;

// PZEM-004T Pins
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17

// General Settings
extern const unsigned long READ_INTERVAL;
extern const unsigned long END_INTERVAL;

#endif

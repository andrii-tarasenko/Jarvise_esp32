#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "ApiClient.h"

extern ApiClient *apiClient;

inline void logRemote(String message) {
  Serial.println(message);
  if (apiClient != nullptr) {
    apiClient->sendLog(message);
  }
}

#endif

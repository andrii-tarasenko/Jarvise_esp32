#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "InverterReader.h"

class ApiClient {
public:
  ApiClient(String const &baseUrl);
  void sendPostRequest(String const &payload);
  void registerDevice(String const &ip);
  void sendPostRequestDetailed(String const &route, String const &payload);
  void sendLog(String const &message);
  bool sendInverterData(InverterData const &data); // Новий метод
private:
  String serverUrl;
};

#endif // API_CLIENT_H

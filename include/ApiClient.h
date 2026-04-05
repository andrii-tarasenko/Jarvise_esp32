#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "InverterReader.h"

class ApiClient {
public:
  ApiClient(String const &baseUrl);

  bool sendInverterData(InverterData const &data);
  void registerDevice(String const &ip);
//  void sendLog(String const &message);
  void sendPostRequest(String const &payload);

private:
  String serverUrl;
  void sendPostRequestDetailed(String const &route, String const &payload);
};

#endif

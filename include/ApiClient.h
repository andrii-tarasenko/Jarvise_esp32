#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>

class ApiClient {
public:
  ApiClient();
  void sendPostRequest(String const &route, String const &method, String &payload);
  void registerDevice(const String &ip, String const &route, String const &method, String &payload);
};

#endif // API_CLIENT_H

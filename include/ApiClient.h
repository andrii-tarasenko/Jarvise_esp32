#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>

class ApiClient {
public:
  ApiClient();
  void sendPostRequest(String const &payload);
  void registerDevice(String const &ip);
  void sendPostRequestDetailed(String const &route, String const &payload);
  void sendLog(String const &message);
};

#endif // API_CLIENT_H

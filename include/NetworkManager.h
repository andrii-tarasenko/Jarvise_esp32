#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>

class NetworkManager {
public:
  NetworkManager();
  void connect();
  bool isConnected();
};

#endif

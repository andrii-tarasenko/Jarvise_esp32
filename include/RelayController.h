#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "Env.h"

class RelayController {
public:
  RelayController();
  void begin();
  void process();
  void setServerState(int channel, bool state);
  bool getServerState(int channel) const;
  bool getCurrentRelayState(int channel) const;

private:
  bool serverState[NUM_RELAYS];
  bool currentRelayState[NUM_RELAYS];
};

#endif // RELAY_CONTROLLER_H

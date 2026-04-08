#include "RelayController.h"
#include "Env.h"

RelayController::RelayController() {
    serverState[0] = false;
    serverState[1] = false;
    serverState[2] = true;
    serverState[3] = false;

    // currentRelayState[0] = false;
    // currentRelayState[1] = false;
    // currentRelayState[2] = true;
    // currentRelayState[3] = false;
}

void RelayController::begin() {
    pinMode(RELAY_PIN_ONE, OUTPUT);
    digitalWrite(RELAY_PIN_ONE, HIGH);

    pinMode(RELAY_PIN_TWO, OUTPUT);
    digitalWrite(RELAY_PIN_TWO, HIGH);

    pinMode(RELAY_PIN_THREE, OUTPUT);
    digitalWrite(RELAY_PIN_THREE, LOW);

    pinMode(RELAY_PIN_FOUR, OUTPUT);
    digitalWrite(RELAY_PIN_FOUR, HIGH);

    pinMode(MOTION_PIN, INPUT_PULLDOWN);

  Serial.println("RelayController initialized. All Relays OFF.");
}

void RelayController::setServerState(int channel, bool state) {
  if (channel >= 1 && channel <= NUM_RELAYS) {
    serverState[channel - 1] = state;
    // Serial.print("RelayController: Server state for Channel ");
    // Serial.print(channel);
    // Serial.print(" updated to ");
    // Serial.println(state ? "ON" : "OFF");
  }
}

bool RelayController::getServerState(int channel) const { 
  if (channel >= 1 && channel <= NUM_RELAYS) {
    return serverState[channel - 1]; 
  }
  return false;
}

// bool RelayController::getCurrentRelayState(int channel) const {
//   if (channel >= 1 && channel <= NUM_RELAYS) {
//     return currentRelayState[channel - 1];
//   }
//   return false;
// }
#include <Arduino.h>
#include <WiFi.h>
#include "NetworkManager.h"
#include "InverterReader.h"
#include "ApiClient.h"
#include "Logger.h"
#include "Env.h"

NetworkManager *networkManager;
InverterReader *inverterReader;
ApiClient *apiClient;

unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 5000;

void setup() {
  Serial.begin(115200);

  delay(3000);

  networkManager = new NetworkManager();
  networkManager->connect();

  inverterReader = new InverterReader(INVERTER_RX_PIN, INVERTER_TX_PIN);
  inverterReader->begin();

  apiClient = new ApiClient(SERVER_HOST);
}

void loop() {
  if (!networkManager->isConnected()) {
    networkManager->connect();
  } //TODO else add cash for data

  inverterReader->process();

  if (millis() - lastSendTime > SEND_INTERVAL) {
    if (networkManager->isConnected()) {
      InverterData data = inverterReader->getData();

      if (data.isValid) {
        if (apiClient->sendInverterData(data)) {
          logRemote("☁️ Data were sent");
        } else {
          logRemote("⚠️ Data were not sent");
        }
      }
    }

    lastSendTime = millis();
  }
}

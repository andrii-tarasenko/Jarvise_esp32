#include <Arduino.h>
#include <WiFi.h>
#include "NetworkManager.h"
#include "InverterReader.h"
#include "ApiClient.h"
#include "Logger.h"
#include "Env.h"
#include "GridSensor.h"
#include "ClimateSensor.h"

NetworkManager *networkManager;
GridSensor *gridSensor;
InverterReader *inverterReader;
ApiClient *apiClient;
ClimateSensor *climateSensor;

unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 5000;

void setup() {
  Serial.begin(115200);
  delay(3000);

  gridSensor = new GridSensor(PZEM_RX_PIN, PZEM_TX_PIN);

  networkManager = new NetworkManager();
  networkManager->connect();

  inverterReader = new InverterReader(INVERTER_RX_PIN, INVERTER_TX_PIN);
  inverterReader->begin();

  apiClient = new ApiClient(SERVER_HOST);

  climateSensor = new ClimateSensor();
  climateSensor->begin();
}

void loop() {
  if (!networkManager->isConnected()) {
    networkManager->connect();
  } //TODO else add cash for data

  inverterReader->process();

  if (millis() - lastSendTime > SEND_INTERVAL) {
    if (networkManager->isConnected()) {

      GridData gData = GridSensor->readData();
      InverterData data = inverterReader->getData();

      if (apiClient->sendESPData(data)) {
        logRemote("☁️ Data were sent");
      } else {
        logRemote("⚠️ Data were not sent");
      }
    }

    lastSendTime = millis();
  }
}

#include <Arduino.h>
#include <WiFi.h>
#include "NetworkManager.h"
#include "InverterReader.h"
#include "ApiClient.h"
#include "LocalWebServer.h"
#include "Logger.h"
#include "Env.h"
#include "GridSensor.h"
#include "ClimateSensor.h"
#include "RelayController.h"

NetworkManager *networkManager;
GridSensor *gridSensor;
InverterReader *inverterReader;
ApiClient *apiClient;
ClimateSensor *climateSensor;
LocalWebServer *localWebServer;
RelayController *relayController;

unsigned long lastSendTime = 0;

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

  relayController = new RelayController();
  relayController->begin();

  localWebServer = new LocalWebServer(SERVER_PORT, relayController, climateSensor);
  localWebServer->begin();
}

void loop() {
  if (!networkManager->isConnected()) {
    networkManager->connect();
  } //TODO else add cash for data

  inverterReader->process();

  if (millis() - lastSendTime > SEND_INTERVAL) {
    if (networkManager->isConnected()) {

      InverterData data = inverterReader->getData();
      GridData gData = gridSensor->readData();

      if (apiClient->sendESPData(data, gData)) {
        logRemote("☁️ Data were sent");
      } else {
        logRemote("⚠️ Data were not sent");
      }
    }

    lastSendTime = millis();
  }

  localWebServer->handleClient();
}

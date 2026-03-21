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
  Serial.println("\n🚀 Jarvise ESP32 Advanced Diagnostic Mode Starting...");

  networkManager = new NetworkManager();
  networkManager->connect();

  inverterReader = new InverterReader(INVERTER_RX_PIN, INVERTER_TX_PIN);
  // Сканер почнеться автоматично в loop() через process()
  
  apiClient = new ApiClient(SERVER_HOST);

  logRemote("🚀 Advanced Scanner Started: Baud (2400/9600) x Polarity (Normal/Inverted)");
}

void loop() {
  if (!networkManager->isConnected()) {
    networkManager->connect();
  }

  inverterReader->process();

  if (millis() - lastSendTime > SEND_INTERVAL) {
    if (networkManager->isConnected()) {
      InverterData data = inverterReader->getData();
      if (data.isValid) {
        apiClient->sendInverterData(data);
      }
    }
    lastSendTime = millis();
  }
}

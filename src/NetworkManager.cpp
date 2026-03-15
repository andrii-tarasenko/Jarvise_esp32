#include "NetworkManager.h"
#include "Env.h"
#include <WiFi.h>

NetworkManager::NetworkManager(){}

void NetworkManager::connect() {
    if (WiFi.status() == WL_CONNECTED)
        return;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
    }

      if (WiFi.status() == WL_CONNECTED) {
         Serial.println("WiFi connected ✅");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("WiFi connect FAILED ❌");
      }
}

bool NetworkManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

#include "NetworkManager.h"
#include "Env.h"
#include <WiFi.h>

NetworkManager::NetworkManager(){}

void NetworkManager::connect() {
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA); // Режим клієнта
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected ✅");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connect FAILED ❌");
    }
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

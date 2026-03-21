#include "ApiClient.h"
#include "Env.h"
#include <WiFi.h>

ApiClient::ApiClient() {}

void ApiClient::sendPostRequest(String const &payload) {
  sendPostRequestDetailed(API_GRID_ROUTE, payload);
}

void ApiClient::sendPostRequestDetailed(String const &route, String const &payload) {
  WiFiClient client;
  if (!client.connect(SERVER_HOST, SERVER_PORT)) {
    Serial.println("❌ Connection to server failed!");
    return;
  }

  String request = String("POST ") + route + " HTTP/1.1\r\n" +
                   "Host: " + VIRTUAL_HOST + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + payload.length() + "\r\n" +
                   "Connection: close\r\n\r\n" + payload;

  client.print(request);

  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println("=== SERVER RESPONSE: " + line);
      break; 
    }
  }

  client.stop();
}

void ApiClient::registerDevice(String const &ip) {
  String payload = "{\"device\":\"invertor_room\",\"ip\":\"" + ip + "\"}";
  sendPostRequestDetailed(API_ROOM_ROUTE, payload);
}

void ApiClient::sendLog(String const &message) {
  String payload = "{\"device\":\"invertor_room\",\"log\":\"" + message + "\"}";
  sendPostRequestDetailed(API_LOGS_ROUTE, payload);
}

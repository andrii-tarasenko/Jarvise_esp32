#include "ApiClient.h"
#include "Env.h"
#include <WiFi.h>

ApiClient::ApiClient() {}


void ApiClient::sendPostRequest(String const &route, String const &method, String &payload) {
  WiFiClient client;

//   if (!client.connect(SERVER_HOST, SERVER_PORT)) {
//     Serial.println("❌ Connection to server failed!");
//     return;
//   }

  String request = method + route + " HTTP/1.1\r\n" +
                   "Host: " + VIRTUAL_HOST + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + payload.length() + "\r\n" +
                   "Connection: close\r\n\r\n" + payload;

  // Serial.println("\n=== SENDING TO SERVER ===");
  // Serial.println(payload);

  // Відправляємо пакет по TCP/IP до вашого бекенду 
  client.print(request);

  // --- Читаємо відповідь від сервера (хоча б перший рядок), щоб звільнити буфер і перевірити статуси ---
  // Serial.print("=== SERVER RESPONSE: ");
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n'); // Читаємо перший рядок (напр., HTTP/1.1 200 OK)
      // Serial.println(line);
      break; 
    }
  }
  Serial.println("=========================\n");

  client.stop(); // Закриваємо з'єднання
}

void ApiClient::registerDevice(const String &ip, String const &route, String const &method, String &payload) {
  WiFiClient client;
//   if (!client.connect(SERVER_HOST, SERVER_PORT)) {
//     Serial.println("❌ Connection to server failed during registration!");
//     return;
//   }

  String request = method + route + " HTTP/1.1\r\n" +
                   "Host: " + VIRTUAL_HOST + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + payload.length() + "\r\n" +
                   "Connection: close\r\n\r\n" + payload;

  Serial.println("\n=== REGISTERING DEVICE ===");
  Serial.println(payload);

  client.print(request);

  Serial.print("=== SERVER RESPONSE: ");
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      break;
    }
  }
 Serial.println("=========================\n");

  client.stop();
}

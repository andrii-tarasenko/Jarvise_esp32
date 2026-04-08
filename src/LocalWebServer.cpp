#include "LocalWebServer.h"
#include <ArduinoJson.h>
#include "RelayController.h"
#include "ClimateSensor.h"

LocalWebServer::LocalWebServer(int port, RelayController *relayCtrl, ClimateSensor *climateSensor)
    : server(port), relayController(relayCtrl), climateSensor(climateSensor) {}

    void LocalWebServer::begin() {
      server.on(RELAY_REQUEST, HTTP_POST, [this]() { this->handlePostRelay(); });
      server.on(CLIMATE_REQUEST, HTTP_POST, [this]() { this->handlePostClimate(); });

      server.onNotFound([this]() { this->handleNotFound(); });

      server.begin();
      Serial.println("LocalWebServer started.");
    }

    void LocalWebServer::handleClient() { server.handleClient(); }

    void LocalWebServer::handlePostRelay() {
      if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body provided\"}");
        return;
      }

      String body = server.arg("plain");
      JsonDocument doc;

      DeserializationError error = deserializeJson(doc, body);

      if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      if (doc.containsKey("state")) {
        bool newState = doc["state"].as<bool>();

        int channel = 1;
        if (doc.containsKey("channel")) {
          channel = doc["channel"].as<int>();
        }

        if (channel < 1 || channel > NUM_RELAYS) {
          server.send(400, "application/json", "{\"error\":\"Invalid channel\"}");
          return;
        }

        relayController->setServerState(channel, newState);

        String response;
        doc.clear();
        doc["message"] = "State updated";
        doc["channel"] = channel;
        doc["current_state"] = newState;
        serializeJson(doc, response);

        server.send(200, "application/json", response);
      } else {
        server.send(400, "application/json",
                    "{\"error\":\"Missing 'state' boolean\"}");
      }
    }

    void LocalWebServer::handleNotFound() {
      server.send(404, "application/json", "{\"error\":\"Not found\"}");
    }

    void LocalWebServer::handlePostClimate() {
      if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body provided\"}");
        return;
      }

      ClimateData cData = climateSensor->readData();
      JsonDocument doc;
      String response;
      doc["temperature"] = cData.temperature;
      doc["humidity"] = cData.humidity;
      doc["pressure"] = cData.pressure;
      serializeJson(doc, response);

      server.send(200, "application/json", response);
    }

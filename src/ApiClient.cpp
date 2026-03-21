#include "ApiClient.h"
#include "Env.h"
#include <WiFi.h>

ApiClient::ApiClient(String const &baseUrl) : serverUrl(baseUrl) {}

void ApiClient::sendPostRequest(String const &payload) {
  sendPostRequestDetailed(API_GRID_ROUTE, payload);
}

bool ApiClient::sendInverterData(InverterData const &data) {
  // Локальне логування для перевірки перед відправкою
  Serial.printf("DEBUG: V_Grid=%.1f F_Grid=%.1f V_Out=%.1f\n", data.grid_voltage, data.grid_freq, data.output_voltage);

  StaticJsonDocument<1024> doc;
  JsonObject root = doc.to<JsonObject>();
  
  root["device"] = "invertor_room";
  root["inv_grid_voltage"] = (float)data.grid_voltage;
  root["inv_grid_freq"] = (float)data.grid_freq;
  root["inv_out_voltage"] = (float)data.output_voltage;
  root["inv_out_freq"] = (float)data.output_freq;
  root["inv_out_va"] = (int)data.output_va;
  root["inv_out_power"] = (int)data.output_power;
  root["inv_out_load_percent"] = (int)data.output_load_percent;
  root["inv_bus_voltage"] = (int)data.bus_voltage;
  root["inv_battery_voltage"] = (float)data.battery_voltage;
  root["inv_battery_charging_current"] = (int)data.battery_charging_current;
  root["inv_battery_capacity"] = (int)data.battery_capacity;
  root["inv_heatsink_temp"] = (int)data.inverter_heatsink_temp;
  root["inv_pv_input_current"] = (float)data.pv_input_current_for_battery;
  root["inv_pv_input_voltage"] = (float)data.pv_input_voltage;
  root["inv_battery_voltage_scc"] = (float)data.battery_voltage_from_scc;
  root["inv_battery_discharge"] = (int)data.battery_discharge_current;
  root["inv_pv_power"] = (float)data.pv_input_power_approx;

  String payload;
  serializeJson(doc, payload);
  sendPostRequestDetailed(API_GRID_ROUTE, payload);
  return true;
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

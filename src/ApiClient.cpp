#include "ApiClient.h"
#include "Env.h"
#include <WiFi.h>

ApiClient::ApiClient(String const &baseUrl) : serverUrl(baseUrl) {}

void ApiClient::sendPostRequest(String const &payload) {
  sendPostRequestDetailed(API_GRID_ROUTE, payload);
}

bool ApiClient::sendESPData(InverterData const &data, GridData const &gData) {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();

  JsonObject grid = root["grid"].to<JsonObject>();
  grid["isValid"] = gData.isValid;
  grid["voltage"] = gData.voltage;
  grid["current"] = gData.current;
  grid["power"] = gData.power;
  grid["energy"] = gData.energy;
  grid["frequency"] = gData.frequency;
  grid["power_f"] = gData.power_f;

  JsonObject inv = root["inverterData"].to<JsonObject>();
  inv["isValid"] = data.isValid;
  inv["grid_voltage"] = data.grid_voltage;
  inv["grid_freq"] = data.grid_freq;
  inv["output_voltage"] = data.output_voltage;
  inv["output_freq"] = data.output_freq;
  inv["output_va"] = data.output_va;
  inv["output_power"] = data.output_power;
  inv["output_load_percent"] = data.output_load_percent;
  inv["bus_voltage"] = data.bus_voltage;
  inv["battery_voltage"] = data.battery_voltage;
  inv["battery_charging_current"] = data.battery_charging_current;
  inv["battery_capacity"] = data.battery_capacity;
  inv["inverter_heatsink_temp"] = data.inverter_heatsink_temp;
  inv["pv_input_current"] = data.pv_input_current_for_battery;
  inv["pv_input_voltage"] = data.pv_input_voltage;
  inv["battery_voltage_scc"] = data.battery_voltage_from_scc;
  inv["battery_discharge_current"] = data.battery_discharge_current;
  inv["pv_power"] = data.pv_input_power_approx;

  String payload;
  serializeJson(doc, payload);

  sendPostRequestDetailed(API_GRID_ROUTE, payload);
  return true;
}

// bool ApiClient::sendInverterData(InverterData const &data) {
//   Serial.printf("DEBUG: V_Grid=%.1f V_Out=%.1f P_Out=%dВт\n",
//                 data.grid_voltage, data.output_voltage, data.output_power);
//
//   StaticJsonDocument<1024> doc;
//   JsonObject root = doc.to<JsonObject>();
//
//   root["device"] = "invertor_room";
//   root["inv_grid_voltage"] = (float)data.grid_voltage;
//   root["inv_grid_freq"] = (float)data.grid_freq;
//   root["inv_out_voltage"] = (float)data.output_voltage;
//   root["inv_out_freq"] = (float)data.output_freq;
//   root["inv_out_va"] = (int)data.output_va;
//   root["inv_out_power"] = (int)data.output_power;
//   root["inv_out_load_percent"] = (int)data.output_load_percent;
//   root["inv_bus_voltage"] = (int)data.bus_voltage;
//   root["inv_battery_voltage"] = (float)data.battery_voltage;
//   root["inv_battery_charging_current"] = (int)data.battery_charging_current;
//   root["inv_battery_capacity"] = (int)data.battery_capacity;
//   root["inv_heatsink_temp"] = (int)data.inverter_heatsink_temp;
//   root["inv_pv_input_current"] = (float)data.pv_input_current_for_battery;
//   root["inv_pv_input_voltage"] = (float)data.pv_input_voltage;
//   root["inv_battery_voltage_scc"] = (float)data.battery_voltage_from_scc;
//   root["inv_battery_discharge"] = (int)data.battery_discharge_current;
//   root["inv_pv_power"] = (float)data.pv_input_power_approx;
//
//   String payload;
//   serializeJson(doc, payload);
//
//   sendPostRequestDetailed(API_GRID_ROUTE, payload);
//   return true;
// }

void ApiClient::sendPostRequestDetailed(String const &route,
                                        String const &payload) {
  WiFiClient client;

  if (!client.connect(SERVER_HOST, SERVER_PORT)) {
    Serial.println("❌ Connection error");
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
      Serial.println("🌐 Server Response: " + line);
      break;
    }
  }

  client.stop();
}

void ApiClient::registerDevice(String const &ip) {
  String payload = "{\"device\":\"invertor_room\",\"ip\":\"" + ip + "\"}";
  sendPostRequestDetailed(API_ROOM_ROUTE, payload);
}

// void ApiClient::sendLog(String const &message) {
//   String payload = "{\"device\":\"invertor_room\",\"log\":\"" + message +
//   "\"}"; sendPostRequestDetailed(API_LOGS_ROUTE, payload);
// }

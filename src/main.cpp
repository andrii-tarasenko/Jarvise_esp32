#include "Env.h"
#include <WiFi.h>
#include "NetworkManager.h"
#include "InverterReader.h"
#include "ApiClient.h"
#include "Logger.h"
#include <ArduinoJson.h>

NetworkManager *networkManager;
InverterReader *inverterReader;
ApiClient *apiClient;

unsigned long lastReadTime = 0;
const unsigned long SEND_INTERVAL = 2000; 

#ifndef UNIT_TEST
void setup()
{
    Serial.begin(115200);
    delay(500);

    networkManager = new NetworkManager();
    networkManager->connect();

    apiClient = new ApiClient();

    if (networkManager->isConnected()) {
        apiClient->registerDevice(WiFi.localIP().toString());
    }

    logRemote("🚀 System starting...");

    inverterReader = new InverterReader(INVERTER_RX_PIN, INVERTER_TX_PIN);
    inverterReader->begin();
}

unsigned long lastFailureTime = 0;
unsigned long lastLoopbackTestTime = 0;

void loop()
{
    networkManager->connect(); 
    inverterReader->process();

    if (millis() - lastReadTime > SEND_INTERVAL) {
        lastReadTime = millis();
        
        if (!networkManager->isConnected()) return;

        InverterData data = inverterReader->getData();
        if (data.isValid) {
            lastFailureTime = millis(); // Reset failure timer
            JsonDocument doc;
            doc["device"] = "invertor_room";
            
            doc["inv_grid_voltage"] = data.grid_voltage;
            doc["inv_grid_freq"] = data.grid_freq;
            doc["inv_out_voltage"] = data.output_voltage;
            doc["inv_out_freq"] = data.output_freq;
            doc["inv_out_va"] = data.output_va;
            doc["inv_out_power"] = data.output_power;
            doc["inv_out_load_percent"] = data.output_load_percent;
            doc["inv_bus_voltage"] = data.bus_voltage;
            doc["inv_battery_voltage"] = data.battery_voltage;
            doc["inv_battery_charging_current"] = data.battery_charging_current;
            doc["inv_battery_capacity"] = data.battery_capacity;
            doc["inv_heatsink_temp"] = data.inverter_heatsink_temp;
            doc["inv_pv_input_current"] = data.pv_input_current_for_battery;
            doc["inv_pv_input_voltage"] = data.pv_input_voltage;
            doc["inv_battery_voltage_scc"] = data.battery_voltage_from_scc;
            doc["inv_battery_discharge"] = data.battery_discharge_current;
            doc["inv_pv_power"] = data.pv_input_power_approx;

            String body;
            serializeJson(doc, body);
            apiClient->sendPostRequest(body);
        } else {
            // Data is invalid, check how long it's been failing
            if (!inverterReader->isScannerActive() && (millis() - lastFailureTime > 5000)) {
                logRemote("⚠️ No data from inverter for 5s. Starting scanner...");
                inverterReader->startScanner();
            }

            // Periodically run loopback test if failing (every 30 seconds)
            if (millis() - lastLoopbackTestTime > 30000) {
                lastLoopbackTestTime = millis();
    // Periodically run diagnostic if no data
//    if (!inverterReader->getData().isValid) {
//        inverterReader->testLoopback();
//    }
}
        }
    }
    
    delay(10);
}
#endif

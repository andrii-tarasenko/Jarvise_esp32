#include <Arduino.h>
#include <string.h>
#include "InverterReader.h"
#include "Logger.h"

#define POLL_INTERVAL 2000

InverterReader::InverterReader(int rx, int tx) : rxPin(rx), txPin(tx) {
  lastCommandTime = 0;
  lastScannerStep = 0;
  scannerIdx = 0;
  scannerActive = true;
  lastData.isValid = false;
}

void InverterReader::begin(uint32_t baud, bool invert) {
  Serial2.end();
  delay(200);
  Serial2.begin(baud, SERIAL_8N1, rxPin, txPin, invert);
  logRemote("🔌 Serial2 config: " + String(baud) + (invert ? " [INVERTED]" : " [NORMAL]"));
}

void InverterReader::process() {
  if (scannerActive) {
    processScanner();
  } else if (millis() - lastCommandTime > POLL_INTERVAL) {
    pollQpigs();
    lastCommandTime = millis();
    
    if (!lastData.isValid) {
      logRemote("⚠️ Connection lost. Restarting scanner...");
      scannerActive = true;
      lastScannerStep = millis();
    }
  }
}

void InverterReader::processScanner() {
  if (millis() - lastScannerStep > 15000) { 
    lastScannerStep = millis();
    
    uint32_t baud = 9600;
    bool invert = false;

    // Спроби: 0=9600N, 1=9600I, 2=2400N, 3=2400I
    switch(scannerIdx) {
      case 1: baud = 9600; invert = true; break;
      case 2: baud = 2400; invert = false; break;
      case 3: baud = 2400; invert = true; break;
      default: baud = 9600; invert = false; break;
    }

    begin(baud, invert);
    logRemote("🔍 Scanner Step [" + String(scannerIdx) + "]: Trying " + String(baud) + (invert ? " Inverted" : " Normal"));
    pollQpigs();
    
    if (lastData.isValid) {
      logRemote("✅ Scanner SUCCESS on Step " + String(scannerIdx));
      scannerActive = false;
    } else {
      scannerIdx = (scannerIdx + 1) % 4; // Цикл по 4-м режимам
    }
  }
}

void InverterReader::pollQpigs() {
  const uint8_t qpigs_raw[] = {0x51, 0x50, 0x49, 0x47, 0x53, 0xB7, 0xA9, 0x0D};
  
  while (Serial2.available()) Serial2.read();
  Serial2.write(qpigs_raw, sizeof(qpigs_raw));

  unsigned long start = millis();
  bool dataReceived = false;
  String rawAscii = "";
  
  while (millis() - start < 5000) {
    if (Serial2.available() > 0) {
      dataReceived = true;
      char c = Serial2.read();
      rawAscii += c;
      if (c == 0x0D) break;
    }
    delay(2);
  }

  if (dataReceived) {
    String hexLog = "";
    for (int j = 0; j < (int)rawAscii.length(); j++) {
      char hex[4];
      sprintf(hex, "%02X ", (uint8_t)rawAscii[j]);
      hexLog += hex;
    }
    logRemote("📡 HEX: " + hexLog);
    logRemote("📡 ASCII: " + rawAscii);
    
    if (rawAscii.indexOf('(') >= 0 || isdigit(rawAscii[0])) {
      int skip = (rawAscii.indexOf('(') >= 0) ? rawAscii.indexOf('(') + 1 : 0;
      String cleanData = rawAscii.substring(skip);
      cleanData.trim();
      
      char buf[cleanData.length() + 1];
      strcpy(buf, cleanData.c_str());
      
      char *p = buf;
      char *str;
      int i = 0;
      
      lastData = InverterData();
      lastData.isValid = true;
      
      while ((str = strtok_r(p, " ", &p)) != NULL) {
        float val = atof(str);
        switch (i) {
          case 0:  lastData.grid_voltage = val; break;
          case 1:  lastData.grid_freq = val; break;
          case 2:  lastData.output_voltage = val; break;
          case 3:  lastData.output_freq = val; break;
          case 4:  lastData.output_va = (int)val; break;
          case 5:  lastData.output_power = (int)val; break;
          case 6:  lastData.output_load_percent = (int)val; break;
          case 7:  lastData.bus_voltage = (int)val; break;
          case 8:  lastData.battery_voltage = val; break;
          case 9:  lastData.battery_charging_current = (int)val; break;
          case 10: lastData.battery_capacity = (int)val; break;
          case 11: lastData.inverter_heatsink_temp = (int)val; break;
          case 12: lastData.pv_input_current_for_battery = val; break;
          case 13: lastData.pv_input_voltage = val; break;
          case 14: lastData.battery_voltage_from_scc = val; break;
          case 15: lastData.battery_discharge_current = (int)val; break;
        }
        i++;
      }
      lastData.pv_input_power_approx = lastData.pv_input_voltage * lastData.pv_input_current_for_battery;
      logRemote("📊 Parsed OK");
    }
  } else {
    logRemote("⚠️ Inverter Timeout (Waiting for response...)");
    lastData.isValid = false;
  }
}

uint16_t InverterReader::calculateCRC(const char *pin, uint8_t len) {
  uint16_t crc = 0x0000;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= (uint16_t)pin[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc;
}

InverterData InverterReader::getData() { return lastData; }

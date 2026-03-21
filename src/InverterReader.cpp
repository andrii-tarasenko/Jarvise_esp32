#include "InverterReader.h"
#include "Env.h"
#include "Logger.h"

#define SLAVE_ID 255
#define INVERTER_BAUD 9600
#define POLL_INTERVAL 2000
#define TIMEOUT 500

InverterReader::InverterReader(int rx, int tx) : rxPin(rx), txPin(tx) {
  lastCommandTime = 0;
  lastReceiveTime = 0;
  waitingForResponse = false;
  lastData.isValid = false;
  lastScannerStepTime = 0;
  scannerActive = false;
  currentBaudIndex = 0; // 9600
  currentSlaveIndex = 0;
}

void InverterReader::begin() {
  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
  // node.begin(SLAVE_ID, Serial2); // Modbus is disabled by default
  logRemote("✅ Inverter Communication established at 9600 baud (PI30)");
}

void InverterReader::process() {
  if (millis() - lastCommandTime > POLL_INTERVAL) {
    pollQpigs();
    lastCommandTime = millis();
  }
}

void InverterReader::startScanner() {
  logRemote("ℹ️ Scanner disabled: Communication already active at 9600.");
}

void InverterReader::processScanner() {
  // Disabled
}

void InverterReader::pollQpigs() {
  // Full packet for QPIGS + CRC + \r in Hex (NO LEADING BRACKET):
  // 51 50 49 47 53 B7 A9 0D
  const uint8_t qpigs_raw[] = {0x51, 0x50, 0x49, 0x47, 0x53, 0xB7, 0xA9, 0x0D};
  
  // Log outgoing hex
  String outHex = "";
  for (size_t i = 0; i < sizeof(qpigs_raw); i++) {
    char h[4];
    sprintf(h, "%02X ", qpigs_raw[i]);
    outHex += h;
  }
  logRemote("➡️ Sending PI30 (HEX): " + outHex);

  // Clear buffer
  while (Serial2.available())
    Serial2.read();

  Serial2.write(qpigs_raw, sizeof(qpigs_raw));

  unsigned long start = millis();
  bool dataReceived = false;
  String rawAscii = "";
  
  while (millis() - start < 5000) {
    if (Serial2.available() > 0) {
      dataReceived = true;
      uint8_t b = Serial2.read();
      if (b >= 32 && b <= 126) rawAscii += (char)b;
      else if (b == 0x0D) rawAscii += "\r";
    }
    if (dataReceived && rawAscii.endsWith("\r")) break;
    delay(10);
  }

  if (dataReceived) {
    logRemote("📡 Inverter Response: " + rawAscii);
    
    // Check if it starts with ( or a digit
    if (rawAscii.startsWith("(") || isdigit(rawAscii[0])) {
      // (226.0 50.0... or 226.0 50.0...
      // If it starts with (, skip it
      int skip = (rawAscii.startsWith("(")) ? 1 : 0;
      char buf[rawAscii.length() + 1];
      strcpy(buf, rawAscii.c_str() + skip);
      
      char *p = buf;
      char *str;
      int i = 0;
      
      lastData.isValid = true;
      while ((str = strtok_r(p, " ", &p)) != NULL) {
        float val = atof(str);
        switch (i) {
        case 0: lastData.grid_voltage = val; break;
        case 1: lastData.grid_freq = val; break;
        case 2: lastData.output_voltage = val; break;
        case 3: lastData.output_freq = val; break;
        case 4: lastData.output_va = (int)val; break;
        case 5: lastData.output_power = (int)val; break;
        case 6: lastData.output_load_percent = (int)val; break;
        case 7: lastData.bus_voltage = (int)val; break;
        case 8: lastData.battery_voltage = val; break;
        case 9: lastData.battery_charging_current = (int)val; break;
        case 10: lastData.battery_capacity = (int)val; break;
        case 11: lastData.inverter_heatsink_temp = (int)val; break;
        case 12: lastData.pv_input_current_for_battery = val; break;
        case 13: lastData.pv_input_voltage = val; break;
        case 14: lastData.battery_voltage_from_scc = val; break;
        case 15: lastData.battery_discharge_current = (int)val; break;
        }
        i++;
      }
      logRemote("🎯 Parsed PI30 data successfully.");
    }
  } else {
    logRemote("⚠️ PI30 Timeout: Absolutely no data received.");
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

void InverterReader::testLoopback() {
  logRemote("🧪 Starting Pin Loopback Test...");
  logRemote("ℹ️ Please short pins " + String(rxPin) + " and " + String(txPin) +
            " now!");

  delay(2000); // Give time to short

  String testMsg = "ESP32_PIN_TEST_" + String(millis());
  Serial2.println(testMsg);

  delay(100);

  if (Serial2.available()) {
    String received = Serial2.readString();
    if (received.indexOf(testMsg) != -1) {
      logRemote("✅ LOOPBACK SUCCESS! Pins " + String(rxPin) + " and " +
                String(txPin) + " are working correctly.");
    } else {
      logRemote(
          "⚠️ Loopback received data, but it didn't match. Check connections.");
    }
  } else {
    logRemote("❌ LOOPBACK FAILED! No data received. Pins " + String(rxPin) +
              "/" + String(txPin) + " might be damaged or not shorted.");
  }
}

void InverterReader::pollRegisters() {
  uint8_t result;

  // Chunk 1: Basic Info & Battery & PV (0x0100 - 0x010F)
  result = node.readHoldingRegisters(0x0100, 16);
  if (result == node.ku8MBSuccess) {
    lastData.isValid = true;
    lastData.battery_capacity = node.getResponseBuffer(0x00); // 0x0100: SOC (%)
    lastData.battery_voltage =
        node.getResponseBuffer(0x01) / 10.0f; // 0x0101: 0.1V
    lastData.battery_charging_current =
        (int16_t)node.getResponseBuffer(0x02) / 10; // 0x0102: 0.1A

    lastData.pv_input_voltage =
        node.getResponseBuffer(0x07) / 10.0f; // 0x0107: 0.1V
    lastData.pv_input_current_for_battery =
        node.getResponseBuffer(0x08) / 10.0f; // 0x0108: 0.1A
    lastData.pv_input_power_approx = node.getResponseBuffer(0x09); // 0x0109: 1W

    logRemote("⚡ Modbus chunk 1 read success.");
  } else {
    logRemote("⚠️ Modbus error 0x0" + String(result, HEX) + " reading 0x0100");
    lastData.isValid = false;
    return; // Don't continue if first chunk failed
  }

  delay(50); // Small pause between requests

  // Chunk 2: AC & Output (0x0200 - 0x0217)
  result = node.readHoldingRegisters(0x0200, 24); // 24 regs to reach 0x0217
  if (result == node.ku8MBSuccess) {
    // 0x0201: AC Input Voltage (0.1V)
    lastData.grid_voltage = node.getResponseBuffer(0x01) / 10.0f;
    // 0x0202: AC Input Frequency (0.01Hz)
    lastData.grid_freq = node.getResponseBuffer(0x02) / 100.0f;

    // 0x0208: AC Output Voltage (0.1V)
    lastData.output_voltage = node.getResponseBuffer(0x08) / 10.0f;
    // 0x0209: AC Output Frequency (0.01Hz)
    lastData.output_freq = node.getResponseBuffer(0x09) / 100.0f;

    // 0x020A: AC Output Active Power (W)
    lastData.output_power = node.getResponseBuffer(0x0A);
    // 0x020B: AC Output Apparent Power (VA)
    lastData.output_va = node.getResponseBuffer(0x0B);

    // 0x020C: Output Load (%)
    lastData.output_load_percent = node.getResponseBuffer(0x0C);

    // 0x0217: Inverter Temperature (1°C)
    // 0x0217 is 23 registers offset from 0x0200
    lastData.inverter_heatsink_temp = (int16_t)node.getResponseBuffer(23);

    logRemote("⚡ Modbus chunk 2 read success.");
  } else {
    logRemote("⚠️ Modbus error 0x0" + String(result, HEX) + " reading 0x0200");
  }
}

InverterData InverterReader::getData() { return lastData; }

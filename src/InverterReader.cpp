#include "InverterReader.h"
#include "Env.h"

// 2400 baud is standard for EASUN/Voltronic RS232
#define INVERTER_BAUD 2400 
#define POLL_INTERVAL 5000 // Query every 5 seconds
#define TIMEOUT 2000       // 2 seconds timeout for response

InverterReader::InverterReader(int rx, int tx)
  : serialPort(1), rxPin(rx), txPin(tx) {
  lastCommandTime = 0;
  lastReceiveTime = 0;
  waitingForResponse = false;
  buffer = "";
  lastData.isValid = false;
}

void InverterReader::begin() {
  serialPort.begin(INVERTER_BAUD, SERIAL_8N1, rxPin, txPin);
  Serial.println("🔌 InverterReader initialized on Serial1 (Pins: RX=" + String(rxPin) + ", TX=" + String(txPin) + ")");
}

// Voltronic standard CRC-16 (XMODEM) calculation
unsigned int InverterReader::calcCRC(const char *cmd) {
  unsigned int crc = 0;
  int i = 0;
  while (cmd[i] != '\0') {
    crc = crc ^ (((unsigned int)cmd[i]) << 8);
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
    i++;
  }
  return crc;
}

void InverterReader::sendCommand(String command) {
  unsigned int crc = calcCRC(command.c_str());
  char crcBuf[3];
  crcBuf[0] = (crc >> 8) & 0xFF; // High byte
  crcBuf[1] = crc & 0xFF;        // Low byte
  crcBuf[2] = '\r';              // Carriage return

  serialPort.print(command);
  serialPort.write((uint8_t)crcBuf[0]);
  serialPort.write((uint8_t)crcBuf[1]);
  serialPort.write((uint8_t)crcBuf[2]);

  Serial.println("=> Sent to Inverter: " + command);
}

void InverterReader::process() {
  // If it's time to poll, and we aren't currently waiting (or a timeout occurred)
  if (millis() - lastCommandTime > POLL_INTERVAL) {
    if (!waitingForResponse || (millis() - lastReceiveTime > TIMEOUT)) {
      if (waitingForResponse) {
         Serial.println("⚠️ Inverter response timeout.");
         lastData.isValid = false; // Mark data as stale if we lost connection
      }
      
      buffer = "";
      waitingForResponse = true;
      lastCommandTime = millis();
      lastReceiveTime = millis();
      sendCommand("QPIGS");
    }
  }

  // Check for incoming data
  while (serialPort.available() > 0) {
    char c = serialPort.read();
    lastReceiveTime = millis();

    if (c == '\r') {
      // End of message
      waitingForResponse = false;
      Serial.println("<= Received from Inverter: " + buffer);
      parseData(buffer);
      buffer = "";
    } else {
      buffer += c;
      // Safeguard against buffer overflow
      if(buffer.length() > 200) buffer = ""; 
    }
  }
}

void InverterReader::parseData(String payload) {
  // Example QPIGS response:
  // (230.0 21.5 230.0 50.0 12.5 2150 2000 48.0 20.0 100 12.0 40.0 1200\r
  
  if (!payload.startsWith("(")) {
    Serial.println("❌ Invalid inverter payload format: " + payload);
    return;
  }

  payload = payload.substring(1);
  
  String tokens[25]; 
  int tokenCount = 0;
  int startIndex = 0;
  for (int i = 0; i <= payload.length(); i++) {
    if (i == payload.length() || payload.charAt(i) == ' ') {
      tokens[tokenCount] = payload.substring(startIndex, i);
      tokenCount++;
      startIndex = i + 1;
      if (tokenCount >= 25) break; 
    }
  }

  if (tokenCount < 15) {
     Serial.println("❌ Not enough data parameters from inverter.");
     return;
  }

  // Populate the struct instead of a JSON document.
  lastData.isValid = true;
  lastData.grid_voltage = tokens[0].toFloat();
  lastData.grid_freq = tokens[1].toFloat();
  lastData.output_voltage = tokens[2].toFloat();
  lastData.output_freq = tokens[3].toFloat();
  lastData.output_va = tokens[4].toInt();
  lastData.output_power = tokens[5].toInt();
  lastData.output_load_percent = tokens[6].toInt();
  lastData.bus_voltage = tokens[7].toInt();
  lastData.battery_voltage = tokens[8].toFloat();
  lastData.battery_charging_current = tokens[9].toInt();
  lastData.battery_capacity = tokens[10].toInt();
  lastData.inverter_heatsink_temp = tokens[11].toInt();
  lastData.pv_input_current_for_battery = tokens[12].toFloat();
  lastData.pv_input_voltage = tokens[13].toFloat();
  lastData.battery_voltage_from_scc = tokens[14].toFloat();
  lastData.battery_discharge_current = tokens[15].toInt();
  lastData.pv_input_power_approx = tokens[13].toFloat() * tokens[12].toFloat();

  Serial.println("⚡ Inverter data parsed and stored in memory.");
}

InverterData InverterReader::getData() {
  return lastData;
}

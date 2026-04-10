#include <Arduino.h>
#include <string.h>
#include "InverterReader.h"
#include "Logger.h"

#define POLL_INTERVAL 2000

InverterReader::InverterReader(int rx, int tx) : rxPin(rx), txPin(tx) {
  lastCommandTime = 0;
  lastData.isValid = false;
}

void InverterReader::begin() {
  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
//  logRemote("✅ Connection with inverter: 9600 bod (OK)");
}

void InverterReader::process() {
  if (millis() - lastCommandTime > POLL_INTERVAL) {
    pollQpigs();
    lastCommandTime = millis();
  }
}

void InverterReader::pollQpigs() {
  // Команда QPIGS з розрахованим CRC16 та символом повернення каретки \r (0x0D)
  // HEX: 51 50 49 47 53 (QPIGS) + B7 A9 (CRC) + 0D (\r)
  const uint8_t qpigs_raw[] = {0x51, 0x50, 0x49, 0x47, 0x53, 0xB7, 0xA9, 0x0D};

  // Очищуємо вхідний буфер Serial, щоб старі байти не заважали новій відповіді
  while (Serial2.available()) Serial2.read();

  // Надсилаємо 8 байтів команди в порт
  Serial2.write(qpigs_raw, sizeof(qpigs_raw));

  unsigned long start = millis();
  bool dataReceived = false;
  String rawAscii = "";

  while (millis() - start < 3000) {
    if (Serial2.available() > 0) {
      dataReceived = true;
      char c = Serial2.read();
      rawAscii += c;
      if (c == 0x0D) break;
    }
    // delay(2);
  }

  if (dataReceived && rawAscii.length() > 20) {
    // Відповідь PI30 зазвичай починається з дужки (
    if (rawAscii.indexOf('(') >= 0 || isdigit(rawAscii[0])) {

      // Знаходимо початок даних після дужки
      int skip = (rawAscii.indexOf('(') >= 0) ? rawAscii.indexOf('(') + 1 : 0;
      String cleanData = rawAscii.substring(skip);
      cleanData.trim(); // Прибираємо пробіли на початку та в кінці

      // Створюємо буфер для розбиття рядка на частини
      char buf[cleanData.length() + 1];
      strcpy(buf, cleanData.c_str());

      char *p = buf;
      char *str;
      int i = 0;

      // Скидаємо структуру даних у нулі перед заповненням
      lastData = InverterData();
      lastData.isValid = true;

      // Розбиваємо рядок за пробілами (strtok_r) і заповнюємо поля структури
      while ((str = strtok_r(p, " ", &p)) != NULL) {
        float val = atof(str); // Перетворюємо ASCII текст у число (float)
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

      // Додатковий розрахунок по сонячних панелях (Вт = В * А)
      lastData.pv_input_power_approx = lastData.pv_input_voltage * lastData.pv_input_current_for_battery;

      logRemote("📊 Інвертор: Power=" + String(lastData.output_power) + "W, Batt=" + String(lastData.battery_voltage) + "V");
    }
  } else {
    // Якщо за 3 секунди відповіді немає - ставимо статус невалідних даних
    lastData.isValid = false;
  }
}

/**
 * Метод для розрахунку CRC (Cyclic Redundancy Check) для PI30.
 * Використовує стандартний алгоритм CRC16-XMODEM.
 */
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

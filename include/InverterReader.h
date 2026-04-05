#ifndef IN_READER_H
#define IN_READER_H

#include <Arduino.h>
#include <stdint.h>
#include <ArduinoJson.h>

/**
 * Структура для зберігання даних інвертора.
 * Містить основні електричні параметри, які ми отримуємо через QPIGS.
 */
struct InverterData {
  bool isValid;                    // Прапор валідності даних (чи успішно пройшов парсинг)
  float grid_voltage;              // Напруга вхідної мережі (AC)
  float grid_freq;                 // Частота вхідної мережі
  float output_voltage;            // Вихідна напруга інвертора (на споживачів)
  float output_freq;               // Вихідна частота
  int output_va;                   // Повна потужність (ВА)
  int output_power;                // Активна потужність (Вт) - реальне споживання
  int output_load_percent;         // Відсоток навантаження
  int bus_voltage;                 // Внутрішня напруга шини DC
  float battery_voltage;           // Поточна напруга акумулятора
  int battery_charging_current;    // Струм зарядки АКБ (А)
  int battery_capacity;            // Заряд АКБ у відсотках
  int inverter_heatsink_temp;      // Температура радіатора інвертора
  float pv_input_current_for_battery; // Струм від сонячних панелей до АКБ
  float pv_input_voltage;          // Напруга сонячних панелей (PV)
  float battery_voltage_from_scc;  // Напруга АКБ згідно з контролером заряду
  int battery_discharge_current;   // Струм розрядки АКБ (А)
  float pv_input_power_approx;     // Розрахункова потужність панелей (V * A)

  // Конструктор за замовчуванням: обнуляє всі поля для запобігання "сміттю" в даних
  InverterData() {
    isValid = false;
    grid_voltage = 0; grid_freq = 0; output_voltage = 0; output_freq = 0;
    output_va = 0; output_power = 0; output_load_percent = 0; bus_voltage = 0;
    battery_voltage = 0; battery_charging_current = 0; battery_capacity = 0;
    inverter_heatsink_temp = 0; pv_input_current_for_battery = 0;
    pv_input_voltage = 0; battery_voltage_from_scc = 0;
    battery_discharge_current = 0; pv_input_power_approx = 0;
  }
};

/**
 * Клас для читання та обробки даних з інвертора через порт RS232.
 */
class InverterReader {
public:
  // Конструктор: приймає піни RX та TX для підключення MAX3232
  InverterReader(int rx, int tx);

  // Ініціалізація Serial2 на швидкості 9600
  void begin();                   

  // Основний метод, який потрібно викликати в loop(): керує таймінгами опитування
  void process();                 

  // Повертає структуру з останніми отриманими даними
  InverterData getData();         

private:
  int rxPin, txPin;               // Збережені номери пінів
  unsigned long lastCommandTime;  // Час останнього успішного запиту
  InverterData lastData;          // Останній зліпок даних

  // Надсилає команду QPIGS та зчитує ASCII відповідь
  void pollQpigs();               

  // Розрахунок контрольної суми (CRC) для протоколу PI30
  uint16_t calculateCRC(const char *pin, uint8_t len); 
};

#endif // IN_READER_H

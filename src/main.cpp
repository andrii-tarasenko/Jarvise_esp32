#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h> // Бібліотека для PZEM-004T

// Wi‑Fi (2.4GHz) =====
const char* WIFI_SSID = "DOM";
const char* WIFI_PASS = "55555555";

// ===== Налаштування сервера =====
const char* SERVER_HOST = "192.168.0.193"; 
const uint16_t SERVER_PORT = 80;
const char* VIRTUAL_HOST = "jarvise.local"; 
const char* API_ROUTE = "/api/v1/grid/readings";

// ===== Налаштування PZEM-004T =====
// RX піде на 16 (TX2), TX піде на 17 (RX2)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// Змінні для збереження попередніх значень (для перевірки змін)
float last_power = -1.0;
float last_voltage = -1.0;

unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 1000; // Опитувати кожну 1 секунду

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED) return;
    
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected ✅");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi connect FAILED ❌");
    }
}

// Універсальний метод відправки POST-запиту
void sendPostRequest(const char* endpoint, const String& payload)
{
    WiFiClient client;
    if (!client.connect(SERVER_HOST, SERVER_PORT))
    {
        Serial.println("❌ Connection to server failed!");
        return;
    }

    String request = String("POST ") + endpoint + " HTTP/1.1\r\n" +
                     "Host: " + VIRTUAL_HOST + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + payload.length() + "\r\n" +
                     "Connection: close\r\n\r\n" +
                     payload;

    Serial.println("\n=== SENDING TO SERVER ===");
    Serial.println(payload);
    
    client.print(request);

    Serial.print("=== SERVER RESPONSE: ");
    while (client.connected() || client.available())
    {
        if (client.available())
        {
            String line = client.readStringUntil('\n');
            Serial.println(line);
            break; // Читаємо тільки перший рядок (статус), щоб не спамити консоль HTML-кодом
        }
    }
    Serial.println("=========================\n");
    
    client.stop();
}

void processPZEMData()
{
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
        return;
    }

    // 1. Читаємо РЕАЛЬНІ дані з датчика
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power(); // Потужність у Ватах
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float power_f = pzem.pf();

    // Перевіряємо, чи датчик взагалі підключений до 220В і чи віддає цифри
    if (isnan(voltage) || isnan(power)) {
        Serial.println("❌ Error reading PZEM (No AC power or disconnected)");
        return;
    }

    // 2. Логіка: відправляти тільки якщо дані суттєво змінились
    // Наприклад: потужність змінилася > 2 Вт, АБО це перший запуск (last_power == -1)
    bool isChanged = false;
    
    if (last_power == -1.0) isChanged = true; 
    if (abs(power - last_power) > 2.0) isChanged = true;
    if (abs(voltage - last_voltage) > 2.0) isChanged = true;

    // 3. Якщо показники змінились — пакуємо в JSON і відправляємо
    if (isChanged)
    {
        Serial.println("⚡ Grid data changed! Preparing payload...");

        // Оновлюємо попередні значення
        last_power = power;
        last_voltage = voltage;

        // 🔥 ОСЬ ТУТ МИ ЗАСЕТИЛИ РЕАЛЬНІ ДАНІ
        JsonDocument doc;
        doc["device"] = "esp32_pzem";
        
        // Реальні дані з мережі:
        doc["grid_power"] = power;  // Твоє головне поле (вже реальне)
        doc["voltage"] = voltage;   // Можеш додати в бекенд, дуже корисна інфа
        doc["current"] = current;   // Сила струму
        doc["energy"] = energy;
        doc["frequency"] = frequency;
        doc["power_f"] = power_f;
        
        // Поки що залишаємо статичні заглушки для полів, яких ще немає фізично
        // (щоб твій Devilbox не видав помилку валідації JSON)
        doc["solar_power"] = 0.0;
        doc["temperature"] = 22.5; 
        doc["humidity"] = 45.0;

        String body;
        serializeJson(doc, body);

        // Відправляємо на сервер
        sendPostRequest(API_ROUTE, body);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    
    connectWiFi();
}

void loop()
{
    // Опитуємо датчик 1 раз на секунду (щоб не перевантажити ESP і не спамити)
    if (millis() - lastReadTime > READ_INTERVAL)
    {
        lastReadTime = millis();
        processPZEMData();
    }
}
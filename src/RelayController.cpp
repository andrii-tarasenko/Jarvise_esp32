#include "RelayController.h"
#include "Env.h"

RelayController::RelayController() {
    serverState[0] = false;
    serverState[1] = false;
    serverState[2] = true;
    serverState[3] = false;

    currentRelayState[0] = false;
    currentRelayState[1] = false;
    currentRelayState[2] = true;
    currentRelayState[3] = false;
}

void RelayController::begin() {
    pinMode(RELAY_PIN_ONE, OUTPUT);
    digitalWrite(RELAY_PIN_ONE, HIGH);

    pinMode(RELAY_PIN_TWO, OUTPUT);
    digitalWrite(RELAY_PIN_TWO, HIGH);

    pinMode(RELAY_PIN_THREE, OUTPUT);
    digitalWrite(RELAY_PIN_THREE, LOW);

    pinMode(RELAY_PIN_FOUR, OUTPUT);
    digitalWrite(RELAY_PIN_FOUR, HIGH);

    pinMode(MOTION_PIN, INPUT_PULLDOWN);

  Serial.println("RelayController initialized. All Relays OFF.");
}

void RelayController::setServerState(int channel, bool state) {
  if (channel >= 1 && channel <= NUM_RELAYS) {
    serverState[channel - 1] = state;
    // Serial.print("RelayController: Server state for Channel ");
    // Serial.print(channel);
    // Serial.print(" updated to ");
    // Serial.println(state ? "ON" : "OFF");
  }
}

bool RelayController::getServerState(int channel) const {
  if (channel >= 1 && channel <= NUM_RELAYS) {
    return serverState[channel - 1];
  }
  return false;
}

bool RelayController::getCurrentRelayState(int channel) const {
 if (channel >= 1 && channel <= NUM_RELAYS) {
   return currentRelayState[channel - 1];
 }
 return false;
}

void RelayController::process() {
  static int stableMotionState = LOW;
  static unsigned long lastMotionDebounceTime = 0;
  static const unsigned long DEBOUNCE_DELAY = 100;

  int currentReading = digitalRead(MOTION_PIN);

  if (currentReading != stableMotionState) {
    if ((millis() - lastMotionDebounceTime) > DEBOUNCE_DELAY) {
      stableMotionState = currentReading;
    }
  } else {
    lastMotionDebounceTime = millis();
  }

  int motionDetected = stableMotionState;

  // static unsigned long lastDebugTime = 0;
  // if (millis() - lastDebugTime > 1000) {
  //   lastDebugTime = millis();
  //   Serial.print("--- DEBUG: Motion Sensor (Pin ");
  //   Serial.print(MOTION_PIN);
  //   Serial.print(") is currently reading: ");
  //   Serial.println(motionDetected == HIGH ? "HIGH (1) [Motion Detected]"
  //                                         : "LOW (0) [No Motion]");
  // }
  // ---------------------

  // Логування зміни станів. Якщо змінився стан руху або стан від сервера - виводимо в консоль всю таблицю прийняття рішень.
  static int lastMotionState = -1;
  static bool lastServerState[NUM_RELAYS] = {false, false, true, false};

  bool logicChanged = false;
  if (motionDetected != lastMotionState) logicChanged = true;
  for(int i=0; i<NUM_RELAYS; i++) {
    if(serverState[i] != lastServerState[i]) logicChanged = true;
  }

  // 2. Блок друку логіки "хто і що вмикає", ЯКЩО стан датчиків змінився
  if (logicChanged) {
    // Serial.println("\n--- RELAYS LOGIC UPDATE ---");
    // Serial.print("Motion Sensor: ");
    // Serial.println(motionDetected == HIGH ? "DETECTED" : "NO MOTION");

    for (int i = 0; i < NUM_RELAYS; i++) {
      // Serial.print("Ch ");
      // Serial.print(i + 1);
      // Serial.print(" ServerState=");
      // Serial.print(serverState[i] ? "ON" : "OFF");

      bool target = false;
      if (i == 0) {
        if (serverState[i]) {
          target = true; // Примусове увімкнення
        } else {
          target = (motionDetected == HIGH); // Робота від датчика руху
        }
      } else {
        // Канали 2,3,4 напряму залежать тільки від стану сервера.
        target = serverState[i];
      }

      // Serial.print(" => Target: ");
      // Serial.println(target ? "ON" : "OFF");
    }
    // Serial.println("---------------------------\n");

    // Оновлюємо попередні стани
    lastMotionState = motionDetected;
    for(int i=0; i<NUM_RELAYS; i++) {
      lastServerState[i] = serverState[i];
    }
  }

  // 3. ФІЗИЧНЕ УВІМКНЕННЯ/ВИМКНЕННЯ РЕЛЕ
  for (int i = 0; i < NUM_RELAYS; i++) {
    bool shouldBeOn = false;

    if (i == 0) {
      if (serverState[i]) {
        shouldBeOn = true; // Примусове увімкнення через сервер
      } else {
        shouldBeOn = (motionDetected == HIGH); // Робота від датчика руху
      }
    } else {
      // Channels 2-4: Тільки стан сервера
      shouldBeOn = serverState[i];
    }

    // Якщо потрібно перемкнути фізичний статус:
    // Порівнюємо shouldBeOn з ПОТОЧНИМ ФІЗИЧНИМ станом, а не з serverState
    if (shouldBeOn != currentRelayState[i]) {
      // Serial.printf("🔄 RELAY DEBUG: Ch %d, Target:%s, PrevPhysical:%s, serverState:%s, motion:%s\n",
      //               i+1, shouldBeOn?"ON":"OFF", serverState[i]?"ON":"OFF",
      //               currentRelayState[i]?"ON":"OFF", (motionDetected == HIGH)?"DETECTED":"NO");
      if (shouldBeOn) {
        digitalWrite(RELAY_PINS[i], LOW); // ON (inverted logic)
        // Serial.print("RelayController: ⚡ Relay ");
        // Serial.print(i + 1);
        // Serial.println(" turned ON");
      } else {
        digitalWrite(RELAY_PINS[i], HIGH); // OFF (inverted logic)
        // Serial.print("RelayController: 🛑 Relay ");
        // Serial.print(i + 1);
        // Serial.println(" turned OFF");
      }
      // Оновлюємо поточний фізичний стан
      currentRelayState[i] = shouldBeOn;
    }
  }
}

#include "Env.h"
#include "NetworkManager.h"
#include "InverterReader.h"

NetworkManager *networkManager;
InverterReader *inverterReader;

unsigned long lastReadTime = 0;


#ifndef UNIT_TEST
void setup()
{
    Serial.begin(115200);
    delay(500);

    networkManager = new NetworkManager();
    networkManager->connect();

    inverterReader = new InverterReader(INVERTER_RX_PIN, INVERTER_TX_PIN);
    inverterReader->begin();
}

void loop()
{
    networkManager->connect(); // Maintain connection
    inverterReader->process();
    delay(10);
}
#endif

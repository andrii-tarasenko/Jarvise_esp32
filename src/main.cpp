#include "Env.h"
#include "NetworkManager.h"

NetworkManager *networkManager;

unsigned long lastReadTime = 0;


void setup()
{
    Serial.begin(115200);
    delay(500);

    networkManager = new NetworkManager();
}

void loop()
{
}

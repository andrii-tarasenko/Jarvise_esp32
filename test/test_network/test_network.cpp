#include <Arduino.h>
#include <unity.h>
#include "NetworkManager.h"

NetworkManager *nm;

void test_network_connection() {
    nm->connect();
    TEST_ASSERT_TRUE(nm->isConnected());
}

void setup() {
    // Wait for hardware to stabilize
    delay(2000);
    UNITY_BEGIN();
    
    nm = new NetworkManager();
    
    RUN_TEST(test_network_connection);
    
    UNITY_END();
}

void loop() {
    // Tests are complete, nothing to do here
}

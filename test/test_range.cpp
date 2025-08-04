// Range Testing Application
// Upload this to test communication range
// Use with one ESP32 as sender, another as receiver

#include <Arduino.h>
#include "../include/communication.h"
#include "../include/config.h"

// Test Configuration
#define TEST_NODE_ID 99
#define TEST_INTERVAL 3000
#define TESTS_PER_DISTANCE 10
#define TEST_DURATION_MINUTES 5

// Test Data Structure
struct RangeTestData {
    uint32_t testId;
    uint32_t timestamp;
    float testDistance;      // Distance in meters
    uint8_t txPower;        // Transmission power
    uint16_t sequenceNum;   // Test sequence number
    uint8_t expectedRSSI;   // Expected RSSI for this distance
} __attribute__((packed));

DroneComm comm(TEST_NODE_ID);
uint32_t testNumber = 0;
uint32_t successfulTests = 0;

// Test distances to try (in meters)
float testDistances[] = {10, 25, 50, 100, 200, 500, 1000, 2000};
uint8_t currentDistanceIndex = 0;
uint8_t testsAtCurrentDistance = 0;

void setup() {
    Serial.begin(115200);
    delay(3000); // Extra time for range test setup
    
    Serial.println("\n" + String("=").repeat(60));
    Serial.println("🎯 DRONE SWARM - RANGE TESTING APPLICATION");
    Serial.println("    Version: " + String(PROJECT_VERSION));
    Serial.println("    Build: " + String(BUILD_DATE) + " " + String(BUILD_TIME));
    Serial.println(String("=").repeat(60));
    
    // Initialize communication with maximum power
    Serial.println("[TEST] Initializing LoRa for range testing...");
    if (!comm.begin()) {
        Serial.println("[TEST] FATAL: Communication initialization failed!");
        while (1) delay(1000);
    }
    
    // Set maximum transmission power for range testing
    comm.setTxPower(20);
    
    Serial.println("\n[TEST] 📋 Range Testing Protocol:");
    Serial.println("[TEST]    • Test at each distance: " + String(TESTS_PER_DISTANCE) + " times");
    Serial.println("[TEST]    • Test interval: " + String(TEST_INTERVAL/1000) + " seconds");
    Serial.println("[TEST]    • Max TX power: 20 dBm");
    Serial.println("[TEST]    • Frequency: " + String(LORA_DEFAULT_FREQUENCY/1E6) + " MHz");
    
    Serial.println("\n[TEST] 📏 Test Distances (meters):");
    for (int i = 0; i < sizeof(testDistances)/sizeof(float); i++) {
        Serial.printf("[TEST]    %d. %.0f meters\n", i+1, testDistances[i]);
    }
    
    Serial.println("\n[TEST] ✅ Range testing ready!");
    Serial.println("[TEST] 🔄 Starting automated range test sequence...");
    Serial.println("[TEST] Move devices to specified distances when prompted\n");
    
    delay(2000);
}

void loop() {
    // Check if we've completed all distances
    if (currentDistanceIndex >= sizeof(testDistances)/sizeof(float)) {
        printFinalResults();
        
        // Wait and restart the test cycle
        delay(30000);
        currentDistanceIndex = 0;
        testsAtCurrentDistance = 0;
        testNumber = 0;
        successfulTests = 0;
        return;
    }
    
    // Prompt for distance setup
    if (testsAtCurrentDistance == 0) {
        promptDistanceSetup();
        delay(10000); // Give time to position devices
    }
    
    // Perform range test
    performRangeTest();
    
    delay(TEST_INTERVAL);
}

void promptDistanceSetup() {
    float distance = testDistances[currentDistanceIndex];
    
    Serial.println("\n" + String("🎯").repeat(20));
    Serial.printf("📏 DISTANCE TEST: %.0f METERS\n", distance);
    Serial.println(String("🎯").repeat(20));
    Serial.printf("[SETUP] Please position devices %.0f meters apart\n", distance);
    Serial.println("[SETUP] Ensure clear line of sight between antennas");
    
    if (distance <= 50) {
        Serial.println("[SETUP] Indoor/close range test");
    } else if (distance <= 200) {
        Serial.println("[SETUP] Outdoor test recommended");
    } else {
        Serial.println("[SETUP] Long range test - use elevated positions");
    }
    
    Serial.printf("[SETUP] Starting %d tests in 10 seconds...\n", TESTS_PER_DISTANCE);
    Serial.println("[SETUP] Watch the receiver for signal quality data");
}

void performRangeTest() {
    testsAtCurrentDistance++;
    testNumber++;
    
    float currentDistance = testDistances[currentDistanceIndex];
    
    // Prepare test data
    RangeTestData testData;
    testData.testId = testNumber;
    testData.timestamp = millis();
    testData.testDistance = currentDistance;
    testData.txPower = 20;
    testData.sequenceNum = testsAtCurrentDistance;
    testData.expectedRSSI = calculateExpectedRSSI(currentDistance);
    
    Serial.printf("\n[TEST %d/%d] Distance: %.0fm, Test: %d/%d\n", 
                  currentDistanceIndex + 1, 
                  (int)(sizeof(testDistances)/sizeof(float)),
                  currentDistance,
                  testsAtCurrentDistance, 
                  TESTS_PER_DISTANCE);
    
    Serial.printf("[TX] Sending range test packet #%lu\n", testNumber);
    Serial.printf("[TX] Expected RSSI: ~%d dBm\n", testData.expectedRSSI);
    
    // Send test message
    bool success = comm.broadcastMessage(MSG_GOSSIP, &testData, sizeof(testData));
    
    if (success) {
        successfulTests++;
        Serial.println("[TX] ✅ Test packet sent successfully");
    } else {
        Serial.println("[TX] ❌ Failed to send test packet");
    }
    
    // Check for any received messages (responses from other nodes)
    DroneMessage receivedMsg;
    unsigned long startTime = millis();
    bool receivedResponse = false;
    
    // Wait up to 2 seconds for a response
    while (millis() - startTime < 2000) {
        if (comm.receiveMessage(receivedMsg)) {
            receivedResponse = true;
            Serial.printf("[RX] 📩 Response received from drone %d\n", receivedMsg.sourceId);
            Serial.printf("[RX] RSSI: %d dBm, SNR: %.1f dB\n", 
                          comm.getRSSI(), comm.getSNR());
            break;
        }
        delay(100);
    }
    
    if (!receivedResponse && testsAtCurrentDistance > TESTS_PER_DISTANCE/2) {
        Serial.println("[RX] ⚠️  No response received - may be out of range");
    }
    
    // Move to next distance if we've completed enough tests
    if (testsAtCurrentDistance >= TESTS_PER_DISTANCE) {
        printDistanceResults();
        currentDistanceIndex++;
        testsAtCurrentDistance = 0;
        delay(5000); // Pause between distance tests
    }
}

uint8_t calculateExpectedRSSI(float distance) {
    // Simplified path loss calculation for 433 MHz
    // RSSI = TX_Power - Path_Loss
    // Path_Loss = 20*log10(distance) + 20*log10(frequency) - 147.55
    
    if (distance < 1) distance = 1; // Avoid log(0)
    
    float pathLoss = 20 * log10(distance) + 20 * log10(433) - 147.55;
    int expectedRSSI = 20 - (int)pathLoss; // 20 dBm TX power
    
    return (uint8_t)max(-120, min(0, expectedRSSI));
}

void printDistanceResults() {
    float distance = testDistances[currentDistanceIndex];
    float successRate = 100.0 * testsAtCurrentDistance / TESTS_PER_DISTANCE;
    
    Serial.println("\n" + String("📊").repeat(20));
    Serial.printf("DISTANCE RESULTS: %.0f METERS\n", distance);
    Serial.println(String("📊").repeat(20));
    Serial.printf("Tests Completed: %d/%d\n", testsAtCurrentDistance, TESTS_PER_DISTANCE);
    Serial.printf("Success Rate: %.1f%%\n", successRate);
    
    if (successRate >= 90) {
        Serial.println("✅ EXCELLENT - Reliable communication");
    } else if (successRate >= 70) {
        Serial.println("✅ GOOD - Acceptable communication");
    } else if (successRate >= 50) {
        Serial.println("⚠️  FAIR - Marginal communication");
    } else {
        Serial.println("❌ POOR - Unreliable communication");
    }
    
    Serial.println(String("📊").repeat(20) + "\n");
}

void printFinalResults() {
    Serial.println("\n" + String("🏆").repeat(30));
    Serial.println("FINAL RANGE TEST RESULTS");
    Serial.println(String("🏆").repeat(30));
    
    Serial.printf("Total Tests: %lu\n", testNumber);
    Serial.printf("Successful Tests: %lu\n", successfulTests);
    Serial.printf("Overall Success Rate: %.1f%%\n", 
                  100.0 * successfulTests / testNumber);
    
    Serial.println("\n📏 Distance Summary:");
    for (int i = 0; i < sizeof(testDistances)/sizeof(float); i++) {
        Serial.printf("   %.0fm: %s\n", testDistances[i], 
                      i < currentDistanceIndex ? "✅ Tested" : "⏳ Pending");
    }
    
    comm.printStats();
    
    Serial.println("\n🔄 Restarting test cycle in 30 seconds...");
    Serial.println(String("🏆").repeat(30) + "\n");
}

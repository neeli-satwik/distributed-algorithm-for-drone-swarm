#include <Arduino.h>
#include "../include/communication.h"

// Configuration
#define NODE_ID 1
#define HEARTBEAT_INTERVAL 2000 // 2 seconds
#define STATS_INTERVAL 10000    // 10 seconds

// Global Objects
DroneComm comm(NODE_ID);

// Timing Variables
unsigned long lastHeartbeat = 0;
unsigned long lastStats = 0;
uint32_t messageCount = 0;

// Function Prototypes
void sendHeartbeat();
void handleReceivedMessage(const DroneMessage& msg);
void printSystemInfo();
String getMessageTypeName(uint8_t type);
String getStatusName(uint8_t status);

void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    Serial.println("\n" + String("=").repeat(50));
    Serial.println("🚁 DRONE SWARM PROJECT - DAY 1 TESTING");
    Serial.println("    NODE: SENDER (ESP32 #1)");
    Serial.println("    ROLE: Communication & Infrastructure");
    Serial.println(String("=").repeat(50));
    
    printSystemInfo();
    
    // Initialize communication
    Serial.println("\n[INIT] Initializing communication system...");
    if (!comm.begin()) {
        Serial.println("[INIT] FATAL: Communication initialization failed!");
        Serial.println("[INIT] Check LoRa module wiring and power supply");
        while (1) {
            delay(1000);
            Serial.print(".");
        }
    }
    
    Serial.println("[INIT] ✅ System ready - Starting communication test...");
    Serial.println("[INIT] Sending heartbeat every 2 seconds");
    Serial.println("[INIT] Listening for incoming messages\n");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Send heartbeat messages
    if (currentTime - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // Check for incoming messages
    DroneMessage receivedMsg;
    if (comm.receiveMessage(receivedMsg)) {
        handleReceivedMessage(receivedMsg);
    }
    
    // Print statistics periodically
    if (currentTime - lastStats >= STATS_INTERVAL) {
        comm.printStats();
        lastStats = currentTime;
    }
    
    delay(100); // Small delay to prevent overwhelming the system
}

void sendHeartbeat() {
    HeartbeatData heartbeat;
    heartbeat.droneId = NODE_ID;
    heartbeat.batteryLevel = 85.5 + (random(-50, 50) / 10.0); // Simulated battery
    heartbeat.latitude = 28.7041 + (random(-100, 100) / 10000.0); // Delhi + offset
    heartbeat.longitude = 77.1025 + (random(-100, 100) / 10000.0);
    heartbeat.status = 0; // OK status
    heartbeat.missionState = 1; // Active
    
    messageCount++;
    
    Serial.printf("\n[TX] 📡 Sending heartbeat #%lu\n", messageCount);
    Serial.printf("[TX]    Battery: %.1f%%\n", heartbeat.batteryLevel);
    Serial.printf("[TX]    Location: (%.6f, %.6f)\n", heartbeat.latitude, heartbeat.longitude);
    Serial.printf("[TX]    Status: %s\n", getStatusName(heartbeat.status).c_str());
    
    if (comm.broadcastMessage(MSG_HEARTBEAT, &heartbeat, sizeof(heartbeat))) {
        Serial.println("[TX] ✅ Heartbeat sent successfully");
    } else {
        Serial.println("[TX] ❌ Failed to send heartbeat");
    }
}

void handleReceivedMessage(const DroneMessage& msg) {
    Serial.printf("\n[RX] 📩 Message received from Drone %d\n", msg.sourceId);
    Serial.printf("[RX]    Type: %s (0x%02X)\n", 
                  getMessageTypeName(msg.messageType).c_str(), msg.messageType);
    Serial.printf("[RX]    Sequence: %d\n", msg.sequenceNumber);
    Serial.printf("[RX]    Timestamp: %lu ms\n", msg.timestamp);
    Serial.printf("[RX]    Data Length: %d bytes\n", msg.dataLength);
    
    // Handle specific message types
    if (msg.messageType == MSG_HEARTBEAT && msg.dataLength == sizeof(HeartbeatData)) {
        HeartbeatData* heartbeat = (HeartbeatData*)msg.data;
        Serial.printf("[RX]    📊 Heartbeat Data:\n");
        Serial.printf("[RX]       Drone ID: %d\n", heartbeat->droneId);
        Serial.printf("[RX]       Battery: %.1f%%\n", heartbeat->batteryLevel);
        Serial.printf("[RX]       Location: (%.6f, %.6f)\n", 
                      heartbeat->latitude, heartbeat->longitude);
        Serial.printf("[RX]       Status: %s\n", getStatusName(heartbeat->status).c_str());
        Serial.printf("[RX]       Mission State: %d\n", heartbeat->missionState);
    }
    
    // Signal quality information
    Serial.printf("[RX]    📶 Signal Quality:\n");
    Serial.printf("[RX]       RSSI: %d dBm\n", comm.getRSSI());
    Serial.printf("[RX]       SNR: %.1f dB\n", comm.getSNR());
    
    // Signal quality assessment
    int rssi = comm.getRSSI();
    String quality;
    if (rssi > -70) quality = "Excellent";
    else if (rssi > -80) quality = "Good";
    else if (rssi > -90) quality = "Fair";
    else quality = "Poor";
    
    Serial.printf("[RX]       Quality: %s\n", quality.c_str());
    Serial.println("[RX] ✅ Message processed successfully\n");
}

void printSystemInfo() {
    Serial.println("\n[INFO] 💻 System Information:");
    Serial.printf("[INFO]    Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("[INFO]    Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("[INFO]    CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("[INFO]    Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("[INFO]    Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("[INFO]    SDK Version: %s\n", ESP.getSdkVersion());
}

String getMessageTypeName(uint8_t type) {
    switch (type) {
        case MSG_HEARTBEAT: return "HEARTBEAT";
        case MSG_GOSSIP: return "GOSSIP";
        case MSG_MUTEX_REQUEST: return "MUTEX_REQUEST";
        case MSG_MUTEX_RESPONSE: return "MUTEX_RESPONSE";
        case MSG_RAFT_VOTE_REQUEST: return "RAFT_VOTE_REQUEST";
        case MSG_RAFT_VOTE_RESPONSE: return "RAFT_VOTE_RESPONSE";
        case MSG_MISSION_UPDATE: return "MISSION_UPDATE";
        case MSG_TARGET_FOUND: return "TARGET_FOUND";
        case MSG_EMERGENCY_STOP: return "EMERGENCY_STOP";
        case MSG_STATUS_REQUEST: return "STATUS_REQUEST";
        case MSG_STATUS_RESPONSE: return "STATUS_RESPONSE";
        default: return "UNKNOWN";
    }
}

String getStatusName(uint8_t status) {
    switch (status) {
        case 0: return "OK";
        case 1: return "WARNING";
        case 2: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

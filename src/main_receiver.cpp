// Use this code for ESP32 #2 (Receiver)
// Copy this to src/main.cpp for the second ESP32

#include <Arduino.h>
#include "../include/communication.h"

// Configuration
#define NODE_ID 2
#define HEARTBEAT_INTERVAL 3000 // 3 seconds (different from sender)
#define STATS_INTERVAL 15000    // 15 seconds

// Global Objects
DroneComm comm(NODE_ID);

// Timing Variables
unsigned long lastHeartbeat = 0;
unsigned long lastStats = 0;
uint32_t messageCount = 0;
uint32_t messagesReceived = 0;

// Function Prototypes
void sendHeartbeat();
void handleReceivedMessage(const DroneMessage& msg);
void printSystemInfo();
void printRangeTestResults();
String getMessageTypeName(uint8_t type);
String getStatusName(uint8_t status);
String formatUptime(unsigned long ms);

void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    Serial.println("\n" + String("=").repeat(50));
    Serial.println("🚁 DRONE SWARM PROJECT - DAY 1 TESTING");
    Serial.println("    NODE: RECEIVER (ESP32 #2)");
    Serial.println("    ROLE: Communication Testing & Analysis");
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
    Serial.println("[INIT] Sending heartbeat every 3 seconds");
    Serial.println("[INIT] Actively listening for all messages");
    Serial.println("[INIT] Performing signal quality analysis\n");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Send heartbeat messages (less frequent than sender)
    if (currentTime - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // Check for incoming messages (primary function)
    DroneMessage receivedMsg;
    if (comm.receiveMessage(receivedMsg)) {
        messagesReceived++;
        handleReceivedMessage(receivedMsg);
    }
    
    // Print comprehensive statistics
    if (currentTime - lastStats >= STATS_INTERVAL) {
        printDetailedStats();
        lastStats = currentTime;
    }
    
    delay(50); // Faster polling for better reception
}

void sendHeartbeat() {
    HeartbeatData heartbeat;
    heartbeat.droneId = NODE_ID;
    heartbeat.batteryLevel = 92.3 + (random(-30, 30) / 10.0); // Simulated battery
    heartbeat.latitude = 28.7041 + (random(-200, 200) / 10000.0); // Delhi + offset
    heartbeat.longitude = 77.1025 + (random(-200, 200) / 10000.0);
    heartbeat.status = 0; // OK status
    heartbeat.missionState = 2; // Listening mode
    
    messageCount++;
    
    Serial.printf("\n[TX] 📡 Sending response heartbeat #%lu\n", messageCount);
    Serial.printf("[TX]    Battery: %.1f%%\n", heartbeat.batteryLevel);
    
    if (comm.broadcastMessage(MSG_HEARTBEAT, &heartbeat, sizeof(heartbeat))) {
        Serial.println("[TX] ✅ Response heartbeat sent");
    } else {
        Serial.println("[TX] ❌ Failed to send response heartbeat");
    }
}

void handleReceivedMessage(const DroneMessage& msg) {
    Serial.printf("\n🎯 [RX #%lu] Message from Drone %d\n", messagesReceived, msg.sourceId);
    Serial.println(String("-").repeat(40));
    
    // Message details
    Serial.printf("📋 Message Info:\n");
    Serial.printf("   Type: %s (0x%02X)\n", 
                  getMessageTypeName(msg.messageType).c_str(), msg.messageType);
    Serial.printf("   Sequence: %d\n", msg.sequenceNumber);
    Serial.printf("   Timestamp: %s\n", formatUptime(msg.timestamp).c_str());
    Serial.printf("   Data Size: %d bytes\n", msg.dataLength);
    Serial.printf("   Checksum: 0x%02X\n", msg.checksum);
    
    // Signal analysis
    int rssi = comm.getRSSI();
    float snr = comm.getSNR();
    Serial.printf("\n📶 Signal Analysis:\n");
    Serial.printf("   RSSI: %d dBm", rssi);
    
    // RSSI interpretation
    if (rssi > -70) Serial.print(" (Excellent)");
    else if (rssi > -80) Serial.print(" (Good)");
    else if (rssi > -90) Serial.print(" (Fair)");
    else if (rssi > -100) Serial.print(" (Poor)");
    else Serial.print(" (Very Poor)");
    
    Serial.printf("\n   SNR: %.1f dB", snr);
    if (snr > 10) Serial.print(" (Excellent)");
    else if (snr > 5) Serial.print(" (Good)");
    else if (snr > 0) Serial.print(" (Fair)");
    else Serial.print(" (Poor)");
    
    // Estimate distance (rough calculation)
    float estimatedDistance = pow(10, (rssi + 30) / -20.0);
    Serial.printf("\n   Est. Distance: %.0f meters\n", estimatedDistance);
    
    // Parse message content
    if (msg.messageType == MSG_HEARTBEAT && msg.dataLength == sizeof(HeartbeatData)) {
        HeartbeatData* heartbeat = (HeartbeatData*)msg.data;
        Serial.printf("\n💓 Heartbeat Details:\n");
        Serial.printf("   Drone ID: %d\n", heartbeat->droneId);
        Serial.printf("   Battery: %.1f%%\n", heartbeat->batteryLevel);
        Serial.printf("   GPS: (%.6f, %.6f)\n", 
                      heartbeat->latitude, heartbeat->longitude);
        Serial.printf("   Status: %s\n", getStatusName(heartbeat->status).c_str());
        Serial.printf("   Mission: State %d\n", heartbeat->missionState);
        
        // Calculate time since message was sent
        unsigned long latency = millis() - msg.timestamp;
        Serial.printf("   Latency: %lu ms\n", latency);
    }
    
    Serial.println(String("-").repeat(40));
    Serial.println("✅ Message processed successfully\n");
}

void printDetailedStats() {
    CommStats stats = comm.getStats();
    
    Serial.println("\n" + String("=").repeat(60));
    Serial.println("📊 DETAILED COMMUNICATION ANALYSIS");
    Serial.println(String("=").repeat(60));
    
    // Basic statistics
    Serial.printf("🔢 Message Statistics:\n");
    Serial.printf("   Messages Sent: %lu\n", stats.messagesSent);
    Serial.printf("   Messages Received: %lu\n", stats.messagesReceived);
    Serial.printf("   Total Messages: %lu\n", messagesReceived);
    
    if (stats.messagesSent > 0) {
        float successRate = 100.0 * stats.messagesSent / (stats.messagesSent + stats.messagesLost);
        Serial.printf("   Success Rate: %.1f%%\n", successRate);
    }
    
    // Signal quality
    Serial.printf("\n📶 Signal Quality:\n");
    Serial.printf("   Last RSSI: %d dBm\n", stats.lastRSSI);
    Serial.printf("   Last SNR: %.1f dB\n", stats.lastSNR);
    
    // System status
    Serial.printf("\n💻 System Status:\n");
    Serial.printf("   Uptime: %s\n", formatUptime(millis()).c_str());
    Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   Node ID: %d\n", comm.getNodeId());
    
    Serial.println(String("=").repeat(60) + "\n");
}

void printSystemInfo() {
    Serial.println("\n💻 System Information:");
    Serial.printf("   Chip: %s (Rev %d)\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("   CPU: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   Flash: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   SDK: %s\n", ESP.getSdkVersion());
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

String formatUptime(unsigned long ms) {
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    return String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
}

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// LoRa Pin Configuration for ESP32
#define LORA_SS     5
#define LORA_RST    14
#define LORA_DIO0   2

// LoRa Communication Settings
#define LORA_FREQUENCY      433E6
#define LORA_TX_POWER       20
#define LORA_BANDWIDTH      125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_CODING_RATE    5
#define LORA_PREAMBLE_LENGTH 8

// Message Types for Drone Swarm
enum DroneMessageType {
    MSG_HEARTBEAT = 0x01,
    MSG_GOSSIP = 0x02,
    MSG_MUTEX_REQUEST = 0x03,
    MSG_MUTEX_RESPONSE = 0x04,
    MSG_RAFT_VOTE_REQUEST = 0x05,
    MSG_RAFT_VOTE_RESPONSE = 0x06,
    MSG_MISSION_UPDATE = 0x07,
    MSG_TARGET_FOUND = 0x08,
    MSG_EMERGENCY_STOP = 0x09,
    MSG_STATUS_REQUEST = 0x0A,
    MSG_STATUS_RESPONSE = 0x0B
};

// Core Message Structure
struct DroneMessage {
    uint8_t messageType;
    uint8_t sourceId;
    uint8_t destinationId; // 0xFF for broadcast
    uint32_t timestamp;
    uint16_t sequenceNumber;
    uint8_t dataLength;
    uint8_t data[32]; // Maximum payload
    uint8_t checksum;
} __attribute__((packed));

// Heartbeat Data Structure
struct HeartbeatData {
    uint8_t droneId;
    float batteryLevel;
    float latitude;
    float longitude;
    uint8_t status; // 0=OK, 1=Warning, 2=Critical
    uint8_t missionState;
} __attribute__((packed));

// Communication Statistics
struct CommStats {
    uint32_t messagesSent;
    uint32_t messagesReceived;
    uint32_t messagesLost;
    int lastRSSI;
    float lastSNR;
    uint32_t uptime;
};

// Communication Interface Class
class DroneComm {
private:
    uint8_t nodeId;
    uint16_t sequenceCounter;
    bool initialized;
    CommStats stats;
    
    uint8_t calculateChecksum(const uint8_t* data, size_t length);
    bool validateChecksum(const DroneMessage& msg);

public:
    DroneComm(uint8_t id);
    
    // Initialization
    bool begin();
    bool isInitialized() const { return initialized; }
    
    // Message Operations
    bool sendMessage(const DroneMessage& msg);
    bool receiveMessage(DroneMessage& msg);
    bool broadcastMessage(DroneMessageType type, const void* data, uint8_t dataLength);
    
    // Configuration
    void setTxPower(int power);
    void setFrequency(long frequency);
    
    // Status & Statistics
    int getRSSI() const;
    float getSNR() const;
    CommStats getStats() const { return stats; }
    uint8_t getNodeId() const { return nodeId; }
    
    // Utility
    void printStats();
    void resetStats();
};

#endif // COMMUNICATION_H

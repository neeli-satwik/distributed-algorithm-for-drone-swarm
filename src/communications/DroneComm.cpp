#include "../../include/communication.h"

DroneComm::DroneComm(uint8_t id) : nodeId(id), sequenceCounter(0), initialized(false) {
    // Initialize statistics
    stats.messagesSent = 0;
    stats.messagesReceived = 0;
    stats.messagesLost = 0;
    stats.lastRSSI = 0;
    stats.lastSNR = 0;
    stats.uptime = 0;
}

bool DroneComm::begin() {
    Serial.println("[COMM] Initializing LoRa communication...");
    
    // Set LoRa pins
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
    // Initialize LoRa
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("[COMM] ERROR: LoRa initialization failed!");
        return false;
    }
    
    // Configure LoRa parameters
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
    
    initialized = true;
    
    Serial.println("[COMM] LoRa initialized successfully");
    Serial.printf("[COMM] Node ID: %d\n", nodeId);
    Serial.printf("[COMM] Frequency: %.1f MHz\n", LORA_FREQUENCY/1E6);
    Serial.printf("[COMM] TX Power: %d dBm\n", LORA_TX_POWER);
    
    return true;
}

bool DroneComm::sendMessage(const DroneMessage& msg) {
    if (!initialized) {
        Serial.println("[COMM] ERROR: Not initialized!");
        return false;
    }
    
    Serial.printf("[COMM] Sending message type 0x%02X to drone %d\n", 
                  msg.messageType, msg.destinationId);
    
    // Start packet transmission
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&msg, sizeof(DroneMessage));
    bool success = LoRa.endPacket();
    
    if (success) {
        stats.messagesSent++;
        Serial.printf("[COMM] Message sent successfully (seq: %d)\n", msg.sequenceNumber);
    } else {
        stats.messagesLost++;
        Serial.println("[COMM] ERROR: Failed to send message");
    }
    
    return success;
}

bool DroneComm::receiveMessage(DroneMessage& msg) {
    int packetSize = LoRa.parsePacket();
    
    if (packetSize == 0) {
        return false; // No packet available
    }
    
    if (packetSize != sizeof(DroneMessage)) {
        Serial.printf("[COMM] WARNING: Invalid packet size: %d bytes\n", packetSize);
        return false;
    }
    
    // Read the packet
    LoRa.readBytes((uint8_t*)&msg, sizeof(DroneMessage));
    
    // Update signal quality stats
    stats.lastRSSI = LoRa.packetRssi();
    stats.lastSNR = LoRa.packetSnr();
    
    // Validate checksum
    if (!validateChecksum(msg)) {
        Serial.println("[COMM] ERROR: Message checksum validation failed");
        stats.messagesLost++;
        return false;
    }
    
    stats.messagesReceived++;
    
    Serial.printf("[COMM] Message received from drone %d (type: 0x%02X, seq: %d)\n", 
                  msg.sourceId, msg.messageType, msg.sequenceNumber);
    Serial.printf("[COMM] Signal: RSSI=%d dBm, SNR=%.1f dB\n", 
                  stats.lastRSSI, stats.lastSNR);
    
    return true;
}

bool DroneComm::broadcastMessage(DroneMessageType type, const void* data, uint8_t dataLength) {
    if (dataLength > 32) {
        Serial.println("[COMM] ERROR: Data too large for message");
        return false;
    }
    
    DroneMessage msg;
    msg.messageType = type;
    msg.sourceId = nodeId;
    msg.destinationId = 0xFF; // Broadcast
    msg.timestamp = millis();
    msg.sequenceNumber = ++sequenceCounter;
    msg.dataLength = dataLength;
    
    // Copy data if provided
    if (data && dataLength > 0) {
        memcpy(msg.data, data, dataLength);
    }
    
    // Calculate and set checksum
    msg.checksum = calculateChecksum((uint8_t*)&msg, sizeof(DroneMessage) - 1);
    
    return sendMessage(msg);
}

uint8_t DroneComm::calculateChecksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool DroneComm::validateChecksum(const DroneMessage& msg) {
    uint8_t calculatedChecksum = calculateChecksum((uint8_t*)&msg, sizeof(DroneMessage) - 1);
    return calculatedChecksum == msg.checksum;
}

void DroneComm::setTxPower(int power) {
    LoRa.setTxPower(power);
    Serial.printf("[COMM] TX Power set to: %d dBm\n", power);
}

void DroneComm::setFrequency(long frequency) {
    LoRa.setFrequency(frequency);
    Serial.printf("[COMM] Frequency set to: %.1f MHz\n", frequency/1E6);
}

int DroneComm::getRSSI() const {
    return stats.lastRSSI;
}

float DroneComm::getSNR() const {
    return stats.lastSNR;
}

void DroneComm::printStats() {
    stats.uptime = millis();
    
    Serial.println("\n=== COMMUNICATION STATISTICS ===");
    Serial.printf("Node ID: %d\n", nodeId);
    Serial.printf("Messages Sent: %lu\n", stats.messagesSent);
    Serial.printf("Messages Received: %lu\n", stats.messagesReceived);
    Serial.printf("Messages Lost: %lu\n", stats.messagesLost);
    Serial.printf("Success Rate: %.1f%%\n", 
                  100.0 * stats.messagesSent / (stats.messagesSent + stats.messagesLost));
    Serial.printf("Last RSSI: %d dBm\n", stats.lastRSSI);
    Serial.printf("Last SNR: %.1f dB\n", stats.lastSNR);
    Serial.printf("Uptime: %.1f seconds\n", stats.uptime / 1000.0);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("===============================\n");
}

void DroneComm::resetStats() {
    stats.messagesSent = 0;
    stats.messagesReceived = 0;
    stats.messagesLost = 0;
    Serial.println("[COMM] Statistics reset");
}

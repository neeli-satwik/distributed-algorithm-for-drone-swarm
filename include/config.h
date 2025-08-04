#ifndef CONFIG_H
#define CONFIG_H

// Project Information
#define PROJECT_NAME "Drone Swarm Distributed Algorithms"
#define PROJECT_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Hardware Configuration
#define MAX_DRONES 5
#define DEFAULT_NODE_ID 1

// LoRa Communication Parameters
#define LORA_DEFAULT_FREQUENCY 433E6
#define LORA_DEFAULT_TX_POWER 20
#define LORA_DEFAULT_BANDWIDTH 125E3
#define LORA_DEFAULT_SF 7
#define LORA_MAX_PAYLOAD_SIZE 32

// Timing Configuration
#define HEARTBEAT_INTERVAL_MS 2000
#define HEARTBEAT_TIMEOUT_MS 6000    // 3x heartbeat interval
#define MESSAGE_TIMEOUT_MS 5000
#define STATS_INTERVAL_MS 10000

// Debug Configuration
#define DEBUG_ENABLED 1
#define DEBUG_COMMUNICATION 1
#define DEBUG_ALGORITHMS 1
#define DEBUG_PERFORMANCE 1

// Mission Parameters
#define MISSION_AREA_SIZE_M 1000     // 1km x 1km area
#define TARGET_DETECTION_RANGE_M 50
#define FORMATION_SPACING_M 100

// Network Configuration
#define MAX_RETRIES 3
#define ACK_TIMEOUT_MS 1000
#define MAX_HOPS 5

// Performance Limits
#define MAX_MESSAGE_RATE_PER_SEC 10
#define MAX_BANDWIDTH_USAGE_PERCENT 80
#define MIN_BATTERY_LEVEL_PERCENT 20

// Pin Definitions (can be overridden in hardware-specific files)
#ifndef LORA_SS
#define LORA_SS 5
#endif

#ifndef LORA_RST  
#define LORA_RST 14
#endif

#ifndef LORA_DIO0
#define LORA_DIO0 2
#endif

// Macros for debugging
#if DEBUG_ENABLED
#define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

// Status LED (if available)
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 2
#endif

#endif // CONFIG_H

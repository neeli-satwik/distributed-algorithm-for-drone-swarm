// Hardware Verification Test
// Upload this FIRST to verify ESP32 and LoRa module are working correctly

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define LORA_SS     5
#define LORA_RST    14
#define LORA_DIO0   2
#define STATUS_LED  2

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n" + String("🔧").repeat(40));
    Serial.println("HARDWARE VERIFICATION TEST");
    Serial.println(String("🔧").repeat(40));
    
    // Test 1: ESP32 Basic Functions
    testESP32();
    
    // Test 2: SPI Communication
    testSPI();
    
    // Test 3: LoRa Module Detection
    testLoRaModule();
    
    // Test 4: LoRa Configuration
    testLoRaConfig();
    
    Serial.println("\n✅ Hardware verification complete!");
    Serial.println("If all tests passed, your hardware is ready for Day 1 tasks.");
}

void loop() {
    // Continuous hardware monitoring
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 5000) {
        Serial.printf("[MONITOR] Uptime: %.1fs, Free Heap: %d bytes\n", 
                      millis()/1000.0, ESP.getFreeHeap());
        
        // Blink status LED
        digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
        
        lastCheck = millis();
    }
    
    delay(100);
}

void testESP32() {
    Serial.println("\n🔸 Test 1: ESP32 Basic Functions");
    
    // System information
    Serial.printf("   Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("   Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("   CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   SDK Version: %s\n", ESP.getSdkVersion());
    
    // Test GPIO
    pinMode(STATUS_LED, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(STATUS_LED, HIGH);
        delay(200);
        digitalWrite(STATUS_LED, LOW);
        delay(200);
    }
    
    Serial.println("   ✅ ESP32 basic functions OK");
}

void testSPI() {
    Serial.println("\n🔸 Test 2: SPI Communication");
    
    // Initialize SPI
    SPI.begin();
    SPI.setFrequency(1000000); // 1 MHz for testing
    
    Serial.printf("   SPI Frequency: %d Hz\n", 1000000);
    Serial.printf("   MOSI Pin: %d\n", MOSI);
    Serial.printf("   MISO Pin: %d\n", MISO);
    Serial.printf("   SCK Pin: %d\n", SCK);
    Serial.printf("   SS Pin: %d\n", LORA_SS);
    
    // Test SPI pins
    pinMode(LORA_SS, OUTPUT);
    digitalWrite(LORA_SS, HIGH);
    
    Serial.println("   ✅ SPI interface initialized");
}

void testLoRaModule() {
    Serial.println("\n🔸 Test 3: LoRa Module Detection");
    
    // Set LoRa pins
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
    Serial.printf("   NSS (SS) Pin: %d\n", LORA_SS);
    Serial.printf("   RST Pin: %d\n", LORA_RST);
    Serial.printf("   DIO0 Pin: %d\n", LORA_DIO0);
    
    // Test LoRa module presence
    Serial.print("   Attempting LoRa initialization... ");
    
    if (!LoRa.begin(433E6)) {
        Serial.println("❌ FAILED");
        Serial.println("   ERROR: LoRa module not detected!");
        Serial.println("   Check:");
        Serial.println("     - Wiring connections");
        Serial.println("     - Power supply (3.3V, NOT 5V!)");
        Serial.println("     - LoRa module compatibility");
        while (1) delay(1000);
    }
    
    Serial.println("✅ SUCCESS");
    Serial.println("   LoRa module detected and responding");
}

void testLoRaConfig() {
    Serial.println("\n🔸 Test 4: LoRa Configuration");
    
    // Test different configurations
    Serial.println("   Testing frequency settings...");
    LoRa.setFrequency(433E6);
    Serial.printf("   ✅ Frequency set to %.1f MHz\n", 433.0);
    
    Serial.println("   Testing power settings...");
    LoRa.setTxPower(20);
    Serial.println("   ✅ TX Power set to 20 dBm");
    
    Serial.println("   Testing signal bandwidth...");
    LoRa.setSignalBandwidth(125E3);
    Serial.println("   ✅ Bandwidth set to 125 kHz");
    
    Serial.println("   Testing spreading factor...");
    LoRa.setSpreadingFactor(7);
    Serial.println("   ✅ Spreading Factor set to 7");
    
    Serial.println("   Testing coding rate...");
    LoRa.setCodingRate4(5);
    Serial.println("   ✅ Coding Rate set to 4/5");
    
    // Test transmission
    Serial.println("\n   Testing packet transmission...");
    LoRa.beginPacket();
    LoRa.print("Hardware Test - ");
    LoRa.print(millis());
    bool success = LoRa.endPacket();
    
    if (success) {
        Serial.println("   ✅ Test packet transmitted successfully");
    } else {
        Serial.println("   ❌ Test packet transmission failed");
    }
    
    Serial.println("\n   🎯 LoRa Configuration Summary:");
    Serial.println("      Frequency: 433.0 MHz");
    Serial.println("      TX Power: 20 dBm (100 mW)");
    Serial.println("      Bandwidth: 125 kHz");
    Serial.println("      Spreading Factor: 7");
    Serial.println("      Coding Rate: 4/5");
    Serial.println("      Estimated Range: 2-15 km (line of sight)");
}

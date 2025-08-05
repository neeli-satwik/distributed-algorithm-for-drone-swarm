// FIRST TEST - Upload this to verify ESP32 works
// This tests ESP32 without LoRa first

#include <Arduino.h>

void setup() {
    // Start serial communication
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    // Set up built-in LED
    pinMode(2, OUTPUT);
    
    Serial.println("\n🚁 ESP32 BASIC TEST");
    Serial.println("===================");
    Serial.println("If you see this message, ESP32 is working!");
    Serial.printf("Chip: %s\n", ESP.getChipModel());
    Serial.printf("CPU Speed: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free Memory: %d bytes\n", ESP.getFreeHeap());
    Serial.println("\nLED should be blinking...");
}

void loop() {
    // Blink the built-in LED
    digitalWrite(2, HIGH);
    Serial.println("LED ON");
    delay(1000);
    
    digitalWrite(2, LOW);
    Serial.println("LED OFF");
    delay(1000);
}

#include <WiFi.h>

const char* SSID = "Conor's A54";
const char* PASSWORD = "buggy2023";

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to network.");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }
    Serial.println("\nConnected");
}

void loop() {
    
}


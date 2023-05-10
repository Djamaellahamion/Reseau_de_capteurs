#include <WiFi.h>

const char* ssid = "ESP32_SFA";
const char* password = "Rose1981";

void setup(){
Serial.begin(115200);
delay(1000);
WiFi.begin(ssid, password);
Serial.println("\nConnecting");

while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
}

Serial.println("\nConnected to the WiFi network");
Serial.print("Local ESP32 IP: ");
Serial.println(WiFi.localIP());
}

void loop(){}

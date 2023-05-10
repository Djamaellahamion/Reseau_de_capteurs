
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>


String MQTT_PUB_TEMP_adressmac= "esp32/bme280/temperature/";
String mac_address=WiFi.macAddress();
int i= 1;



void setup() {
  // put your setup code here, to run once:
  MQTT_PUB_TEMP_adressmac.concat( String(mac_address).c_str());
  String s_node="Node";
  s_node.concat(i);
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP MAC Address:  ");
  Serial.println(MQTT_PUB_TEMP_adressmac);
  Serial.println(s_node);
}

void loop() {
  // put your main code here, to run repeatedly:String mac_address=WiFi.macAddress();


}

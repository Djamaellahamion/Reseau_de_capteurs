#define GATEWAY
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LoRa.h>
#include "LoRa_Config.h"
 
const char* ssid = "ESP32_SFA";
const char* password =  "Rose1981";
const char* mqttServer = "156.18.46.123";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";

const String batiment = "TMM(LTDS)";
const String salle = "353";
const String Post = batiment + "/"+ salle; // c'est le post publié sur le broker
const char* sensorType = "BME280";
 
WiFiClient espClient;
PubSubClient client(espClient);

byte Node =NODE1;   //Node de reception
char message_recu[150];   //stockage du message LoRa reçu et transmis en MQTT

void onReceive(int packetSize) {
  if (packetSize == 0) return;
  // if there's no packet, return
  // read packet header bytes:
  byte recipient = LoRa.read();  // recipient address = Master
  byte sender = LoRa.read();     // sender address = Client
  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  strncpy(message_recu, message.c_str(), sizeof(message_recu)); //conversion string en char
  Serial.print("Gateway Receive: ");
  Serial.print(message);
  Serial.println(" with Rssi: ");
}

void setup() {
  set_LoRa();
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(salle.c_str(), mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

}

void loop() {
  if(runEvery(2500)){
    String message = "HeLoRa World! ";
    message += "I'm a Gateway! ";
    message += millis();
    LoRa_sendMessage(message, MASTERNODE, Node);
    if (client.publish(Post.c_str(), message_recu) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
 
  client.loop();
  Serial.println("-------------");
    
  }
}
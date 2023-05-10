/*
   Serveur LoRa
*/

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// déclare l'accéléromètre MMA8451
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

Adafruit_MMA8451 mma = Adafruit_MMA8451();

#define OLED_SDA 21     // Attention: Les broches SDA et SCL de l'affichage OLED sont inversées sur le dessin de LilyGO et sur GitHub.
#define OLED_SCL 22 
#define OLED_RST 12     // Note: Le TTGO Lora32 v2 n'utilise pas le signal reset, mais la librairie Adafruit_SSD1306, oui.
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels



const int ledPin = 25;    // the number of the LED pin
//Setup de l'écran :
const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;       // LoRa radio reset
const int DIO = 26;         // change for your board; must be a hardware interrupt pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
byte localAddress= 0x00;     // address of this device
byte destination = 0x00;      // destination to send to
int turn = 1;
unsigned long previousMillis = 0;
const long interval = 1000;
String reponse;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Serveur");
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) 
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("LoRa");
  display.setCursor(0,30);
  display.print("Serveur");
  display.display();

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, DIO); // set CS, reset, IRQ pin

  if (!LoRa.begin(868.5E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125000);
  LoRa.onReceive(onReceive);
  LoRa.receive();
    
}

void loop() {
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) < 1000) {
    
    if(turn==1){
      Serial.println("Envoie de la Requete1");
      envoie_message_LoRa("C1");
      //Attente_de_reponse();
      turn=2;
    }else if(((currentMillis - previousMillis) > 1000) && ((currentMillis - previousMillis) < 2000)) {
      if(turn==2){
      Serial.println("Envoie de la Requete2");
      envoie_message_LoRa("C2");
      //Attente_de_reponse();
      turn=3;
      }
    }else if((currentMillis - previousMillis) > 6000){
      if(turn=3){
      Serial.println("Envoie de la Requete3");
      envoie_message_LoRa("C3");
      Attente_de_reponse();
      turn=1;
      previousMillis = currentMillis;
      }
    }

  }


}
void envoie_message_LoRa(String message){
  Serial.println("Envoie du message: ");
  LoRa.beginPacket();
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.print(message.length());
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Message envoyé ");
}
// void Attente_de_reponse(){
//   int packetSize = LoRa.parsePacket();
//   if (packetSize){
//     // received a packet
//     Serial.println("Packet reçu sur le Serveur");
//     // read and print packet
//     display.clearDisplay();
//     display.setCursor(0,0);
//     while (LoRa.available()) 
//     {
//       display.setTextSize(2);
//       //Serial.println(LoRa.readStringUntil(','));
//       reponse = LoRa.readString();
//       reponse = reponse.substring(5);
//       display.print(reponse);
//       Serial.println(reponse);

//     }
//     display.display();
    

//   }
// }

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
 
  // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();  // sender address
  byte incomingMsgId = LoRa.read(); // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read(); 
  }
  Serial.print("Gateway Receive: ");
  Serial.println("Packet reçu sur le Serveur : " + incoming);
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print(incoming);
  display.display();

}

/*
  Système Alarmes Récepteurs LTDS  
*/

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 21     // Attention: Les broches SDA et SCL de l'affichage OLED sont inversées sur le dessin de LilyGO et sur GitHub.
#define OLED_SCL 22 
#define OLED_RST 12     // Note: Le TTGO Lora32 v2 n'utilise pas le signal reset, mais la librairie Adafruit_SSD1306, oui.
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;       // LoRa radio reset
const int irqPin = 26;         // change for your board; must be a hardware interrupt pin

const int ledPin = 25;    // the number of the LED pin

String message;
String reponse="Hello from TTGO";
String Json="{TTGO:1, Salle: 1, donnees : 1}";
byte localAddress= 0x65;     // address of this device
byte destination = 0x31;      // destination to send to

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() 
{
  Serial.begin(9600);
  while (!Serial);

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
  display.print("LTDS Alarm ");
  display.setCursor(0,30);
  display.print("Etat RAS ");
  display.display();

  Serial.println("LoRa Receiver");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin

  if (!LoRa.begin(868.5E6)) 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125000);

  
// initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);  
}

void loop() 
{
  
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    // received a packet
    Serial.println("Received packet '");

    // read and print packet
    display.clearDisplay();
    display.setCursor(0,0);
    while (LoRa.available()) 
    {
      display.setTextSize(2);
      //Serial.println(LoRa.readStringUntil(','));
      message = LoRa.readString();
      message = message.substring(5);
      display.print(message);
      Serial.print(message);
    }
    // print RSSI of packet
    display.setTextSize(1);
    display.setCursor(0,55);
    display.print("RSSI = ");
    display.println(LoRa.packetRssi());
    display.display();
    digitalWrite(ledPin, HIGH);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    
    
    delay(3000); // on attend 3 secondes avant d'envoyer le message de confirmation de bonne réception

    Confirm_Alarm();
  }
}

void Confirm_Alarm(){
  Serial.println("Sending confirmation: ");
 // send packet
  LoRa.beginPacket();
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.print(150);
  //LoRa.print(reponse);
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Confirmation sent ");
  Serial.println(reponse.length());
}
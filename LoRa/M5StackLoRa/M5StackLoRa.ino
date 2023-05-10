/*
  Système Alarmes Récepteurs LTDS  
*/
#include "M5Core2.h"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>




const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;       // LoRa radio reset
const int irqPin = 26;         // change for your board; must be a hardware interrupt pin

const int ledPin = 25;    // the number of the LED pin


void setup() 
{
  Serial.begin(9600);
  while (!Serial);
  M5.begin();
  M5.Lcd.fillScreen(BLACK); //Set the screen background color to black.
  M5.Lcd.setTextColor(GREEN , BLACK); //Sets the foreground color and background color of the displayed text.
  M5.Lcd.setTextSize(2);

  
  
  M5.Lcd.setCursor(0, 175);
  M5.Lcd.printf("LTDS Alarm ");
  M5.Lcd.setCursor(0,30);
  M5.Lcd.print("Etat RAS ");

  Serial.println("LoRa Receiver");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin

  if (!LoRa.begin(868E6)) 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(8);
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
    M5.Lcd.clearDisplay();
    M5.Lcd.setCursor(0,0);
    while (LoRa.available()) 
    {
      M5.Lcd.setTextSize(2);
      M5.Lcd.print(char(LoRa.read()));
    }
    // print RSSI of packet
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0,55);
    M5.Lcd.print("RSSI = ");
    M5.Lcd.println(LoRa.packetRssi());
    digitalWrite(ledPin, HIGH);
    delay(3000);
    //on attend 3 secondes avant d'envoyer le message de confirmation de bonne réception
    Confirm_Alarm();
  }
}

void Confirm_Alarm(){
  Serial.println("Sending confirmation: ");
 // send packet
  LoRa.beginPacket();
  LoRa.println("Alarm");
  LoRa.print("received");
  LoRa.endPacket();
}

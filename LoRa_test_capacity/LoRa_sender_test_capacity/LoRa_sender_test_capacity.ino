/*
   Système alarmes Mobiles LTDS
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
const int irqPin = 26;         // change for your board; must be a hardware interrupt pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//int counter = 0;
String message;

byte localAddress= 0x31;     // address of this device
byte destination = 0x65;      // destination to send to

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Emetteur");
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) 
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(25,0);
  display.print("LoRa Emetteur");
  display.display();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(2000);
  digitalWrite(ledPin, LOW);

  Serial.print("Je vais envoyé le message dans ");
  Serial.print(interval/1000);
  Serial.println("secondes");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("Envoie dans ");
  display.print(interval);
  display.print("ms");

  display.display();

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin

  if (!LoRa.begin(868.5E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setCodingRate4(1);
  LoRa.setSignalBandwidth(125000);
  LoRa.setPreambleLength(4);
  
  //valeur aléatoire
  //message=random(0, 40);
  message=3;
    
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //affichage de la valeur aléaitoire
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print(message);
    display.display();
    envoie_message();
  }


}
void envoie_message(){
  Serial.println("Envoie du message: ");
 // send packet
  LoRa.beginPacket();
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.print(150);
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Message envoyé ");
}


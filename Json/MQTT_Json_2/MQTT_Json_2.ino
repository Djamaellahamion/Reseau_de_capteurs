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

const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;       // LoRa radio reset
const int irqPin = 26;         // change for your board; must be a hardware interrupt pin

const int buttonPin = 4;  // the number of the pushbutton pin
const int ledPin = 25;    // the number of the LED pin
// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status
int Accel_Count = 0 ; // compteur servant à l'accéléromètre
// définition de la pin du haut-parleur
const int TONE_OUTPUT_PIN = 19;

// The ESP32 has 16 channels which can generate 16 independent waveforms
// We'll just choose PWM channel 0 here
const int TONE_PWM_CHANNEL = 0; 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

int counter = 0;

void setup() {
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
  display.print("D5_Bis ON ");
  display.display();

  Serial.println("LoRa Sender");

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
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(ledPin, HIGH);
  delay(2000);
  digitalWrite(ledPin, LOW);
  
  // partie accéléromètre
//  Serial.println("Adafruit MMA8451 test!");
//  
//
//  if (! mma.begin()) {
//    Serial.println("Couldnt start");
//    while (1);
//  }
//  Serial.println("MMA8451 found!");
//  
//  mma.setRange(MMA8451_RANGE_2_G);
//  
//  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
//  Serial.println("G");

  
  // ledcAttachPin(uint8_t pin, uint8_t channel);
  ledcAttachPin(TONE_OUTPUT_PIN, TONE_PWM_CHANNEL);
    
}

void loop() {
// read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    // envoie l'alarme
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,25);
    display.print("Alarm Sent");
    display.display();
    alarm();
    delay(10000);
     digitalWrite(ledPin, LOW);
     display.clearDisplay();
     display.display();
      //delay(2000);
    
 

//  int confirmation = LoRa.parsePacket();
//    if (confirmation) 
//    {
//    // received a packet
//    Serial.println("Retour Centrale");
//
//    // read and print packet
//    display.clearDisplay();
//    display.setCursor(0,15);
//    while (LoRa.available()) 
//      {
//        display.print(char(LoRa.read()));
//      }
//    display.setTextSize(1);
//    display.setCursor(0,55);
//    display.print("RSSI = ");
//    display.println(LoRa.packetRssi());
//    display.display(); 
//    Tone();
        
 //   }
delay(5000); 
  // Serial.print("Sending packet: ");
  // Serial.println(counter);

  // // send packet
  // LoRa.beginPacket();
  // LoRa.print("D5_bis OK #");
  // LoRa.print(counter);
  // LoRa.endPacket();

  // counter++;

  // delay(5000);
}
// Ecrit le message d'alarme à envoyer
void alarm() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  
  LoRa.println("Alarme");
  LoRa.println("D5_Bis");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
}

void Tone() {
  // Plays the middle C scale
  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_C, 4);
  delay(500);
  ledcWriteTone(TONE_PWM_CHANNEL, 800);
  delay(500);
}

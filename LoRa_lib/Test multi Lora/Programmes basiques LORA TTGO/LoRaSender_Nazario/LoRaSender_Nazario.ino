/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/ttgo-lora32-sx1276-arduino-ide/
*********/

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 868.5E6

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 12
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//packet counter
int counter = 0;
byte sender;

byte MasterNode = 0x04;     
byte Node1 = 0x05;

String outgoing;              // outgoing message
String message;
byte msgCount = 0;            // count of outgoing messages

// Tracks the time since last event fired
unsigned long previousMillis=0;
unsigned long int previoussecs = 0; 
unsigned long int currentsecs = 0; 
unsigned long currentMillis = 0;
int Secs = 0; 
 
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();
  
  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSyncWord(0xF3);

  sender = LoRa.read();
  Serial.println(sender,HEX);
  byte recipient = LoRa.read();
  Serial.println(recipient,HEX);
  
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa sender");
  display.setCursor(70,10);
  display.print(sender,HEX);
  display.setCursor(0,20);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void loop() {
  if (millis() - lastSendTime > interval){
    message = "HELLO";
    sendMessage(message,MasterNode,Node1);
    //delay(50);
    lastSendTime = millis();
  }
}

void sendMessage(String outgoing, byte MasterNode, byte otherNode){
  LoRa.beginPacket();                   // start packet
  LoRa.write(otherNode);              // add destination address
  LoRa.write(MasterNode);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID

}

  // Serial.print("Sending packet: ");
  // Serial.println(counter);

  // //Send LoRa packet to receiver
  // LoRa.beginPacket();
  // LoRa.print("C2 ");
  // LoRa.print(counter);
  // LoRa.endPacket();
  
  // display.clearDisplay();
  // display.setCursor(0,0);
  // display.println("LORA SENDER");
  // display.setCursor(80,0);
  // display.println(sender,HEX);
  // display.setCursor(0,20);
  // display.setTextSize(1);
  // display.print("LoRa packet sent.");
  // display.setCursor(0,30);
  // display.print("C2:");
  // display.setCursor(50,30);
  // display.print(counter);      
  // display.display();

  // counter++;
  
  // delay(1000);


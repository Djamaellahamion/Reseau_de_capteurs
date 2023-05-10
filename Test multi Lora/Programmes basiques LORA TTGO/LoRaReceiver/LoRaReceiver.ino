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
#define RST 12
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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;
int counter = 0;
byte receiver;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte MasterNode = 0xFF;     
byte Node1 = 0xBB;

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
  display.print("LORA RECEIVER ");
  display.display();

  Serial.println("LoRa Receiver Test");
  
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  // //    // Change sync word (0xF3) to match the receiver
  // // // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // // // ranges from 0-0xFF
  // // LoRa.setSyncWord(0xF3);
  // LoRa.setFrequency(868.5E6);
  // LoRa.setSpreadingFactor(7);
  // LoRa.setSignalBandwidth(125E3);
  // LoRa.setCodingRate4(5);
  // LoRa.setSyncWord(0x12);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSyncWord(0xF4);
  receiver = LoRa.read();

  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();  
}

void loop() {
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
 
  // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address
  Serial.print(recipient, HEX);
  byte sender = LoRa.read();  // sender address
  Serial.print("---");          
  Serial.print(sender,HEX);
  byte incomingMsgId = LoRa.read(); // incoming msg ID
  Serial.print("---");    
  Serial.print(incomingMsgId);
  byte incomingLength = LoRa.read();    // incoming msg length
  Serial.print("---");
  Serial.print(incomingLength, HEX);
 
  String incoming = "";
 
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
 
  if (incomingLength != incoming.length()) {   // check length for error
   // Serial.println("error: message length does not match length");
   ;
    return;                             // skip rest of function
  }
 
  // if the recipient isn't this device or broadcast,
  // if (recipient != Node1 && recipient != MasterNode) {
  //   //Serial.println("This message is not for me.");
  //   ;
  //   return;                             // skip rest of function
  // }

  String mensaje = String(recipient)+"-"+String(sender)+"-"+String(incomingMsgId)+"-"+String(incomingLength)+"-"+incoming;

  int rssi = LoRa.packetRssi();

  Serial.print("---"); 
  Serial.println(incoming);
  Serial.println()

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("LORA RECEIVER");
  display.setCursor(100,0);
  display.println(sender,HEX);
  display.setCursor(0,20);
  display.print("Received packet:");
  display.setCursor(0,30);
  display.print(mensaje);
  display.setCursor(0,40);
  display.print("RSSI:");
  display.setCursor(30,40);
  display.print(rssi);
  display.display();   
 
  
}




void Receiving_Packet(){
    //try to parse packet
  Serial.println("Received packet OK ");
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.print("Received packet from ");

    //read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rssi);

   // Dsiplay information
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(100,0);
   display.println(receiver,HEX);
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(LoRaData);
   display.setCursor(0,40);
   display.print("RSSI:");
   display.setCursor(30,40);
   display.print(rssi);
   display.display();   
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

void Sending_Packet(){
  Serial.print("Sending packet to C2: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("C2 ");
  LoRa.print(counter);
  LoRa.endPacket();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SERVER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("LoRa packet sent.");
  display.setCursor(0,30);
  display.print("C2:");
  display.setCursor(50,30);
  display.print(counter);      
  display.display();

  counter++;
  //Receiving_Packet();
  
  delay(1000); 
}
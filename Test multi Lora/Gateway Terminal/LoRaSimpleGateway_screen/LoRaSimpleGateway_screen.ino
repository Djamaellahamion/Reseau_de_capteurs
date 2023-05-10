/*
  LoRa Simple Gateway/Node Exemple

  This code uses InvertIQ function to create a simple Gateway/Node logic.

  Gateway - Sends messages with enableInvertIQ()
          - Receives messages with disableInvertIQ()

  Node    - Sends messages with disableInvertIQ()
          - Receives messages with enableInvertIQ()

  With this arrangement a Gateway never receive messages from another Gateway
  and a Node never receive message from another Node.
  Only Gateway to Node and vice versa.

  This code receives messages and sends a message every second.

  InvertIQ function basically invert the LoRa I and Q signals.

  See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
  for more on InvertIQ register 0x33.

  created 05 August 2018
  by Luiz H. Cassettari
*/

#include <SPI.h>  // include libraries
#include <LoRa.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
//OLED pins
#define OLED_SDA 21  // Attention: Les broches SDA et SCL de l'affichage OLED sont inversées sur le dessin de LilyGO et sur GitHub.
#define OLED_SCL 22
#define OLED_RST 12       // Note: Le TTGO Lora32 v2 n'utilise pas le signal reset, mais la librairie Adafruit_SSD1306, oui.
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels


const long frequency = 868.5E6;  // LoRa Frequency

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

int counter = 0;
int const nbNodes = 6;
int i = 1;  //node 1
bool received = false;
byte MasterNode = 0xFF;  //c'est moi
byte Node1 = 0xA1;
byte Node2 = 0xA2;
byte Node3 = 0xA3;
byte Node4= 0xA4;
byte Node5=0xA5;
byte Nodes[nbNodes] = { MasterNode, Node1, Node2, Node3, Node4,Node5 };
byte Node = Nodes[1];  //le premier Node auquel le master envoie
byte PreviousNode = Node;
//Display object for oled screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  Serial.begin(115200);  // initialize serial
  while (!Serial)
    ;
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA GATEWAY ");
  display.display();
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);


  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ;  // if failed, do nothing
  }


  LoRa.setSyncWord(0x3F);
  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  display.setCursor(0, 10);
}

void loop() {
  if (runEvery(2500)) {  // repeat every 5000 millis

    String message = "HeLoRa World! ";
    message += "I'm a Gateway! ";
    message += millis();
    if (Node != Nodes[i]) {
      String Snode = "Node";
      Snode.concat(i);
      Serial.print("Je n'ai rien reçu du ");
      Serial.println(Snode);
    }
    i++;
    i = i % nbNodes;
    if (i == 0) { i = 1; }
    Node = Nodes[i];
    LoRa_sendMessage(message, MasterNode, Node);  // send a message mettre Node2 et le client n'envoie rien
    PreviousNode = Node;
    Serial.println("Send Message!");
  }
}

void LoRa_rxMode() {
  LoRa.disableInvertIQ();  // normal mode
  LoRa.receive();          // set receive mode
}

void LoRa_txMode() {
  LoRa.idle();            // set standby mode
  LoRa.enableInvertIQ();  // active invert I and Q signals
}

void LoRa_sendMessage(String message, byte MasterNode, byte otherNode) {
  LoRa_txMode();           // set tx mode
  LoRa.beginPacket();      // start packet
  LoRa.write(otherNode);   // add destination address
  LoRa.write(MasterNode);  // add sender address
  LoRa.print(message);     // add payload
  LoRa.endPacket(true);    // finish packet and send it
  counter++;
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;
  display.print("reçu");
  display.display();
  // if there's no packet, return
  // read packet header bytes:
  byte recipient = LoRa.read();  // recipient address = Master
  byte sender = LoRa.read();     // sender address = Client
  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }



  received = true;
  Serial.print("Gateway Receive: ");
  Serial.print(message);
  Serial.print(" with Rssi: ");
  Serial.println(LoRa.packetRssi());
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

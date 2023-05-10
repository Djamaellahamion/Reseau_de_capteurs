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

#include <SPI.h>              // include libraries
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const long frequency = 868.5E6;  // LoRa Frequency

const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;        // LoRa radio reset
const int irqPin = 26;          // change for your board; must be a hardware interrupt pin

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 12
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);



void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(csPin, resetPin, irqPin);

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
  display.print("LORA GATEWAY ");
  display.display();


  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
 
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125000);

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
}

void loop() {
  if (runEvery(5000)) { // repeat every 5000 millis

    String message = "HeLoRaWord";
    message += "I'm a Gateway! ";
    message += millis();
    LoRa_sendMessage(message); // send a message

    Serial.print("Send Message!");
    Serial.print("--");
    Serial.println(message);

  
  }
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
  
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  String message="";
  while(LoRa.available()){
    message+=(char)LoRa.read();
  }
  Serial.print("Gateway Receive : ");
  Serial.println(message);
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}


// void Display(){
//   display.clearDisplay();
//   display.setCursor(0,0);
//   display.print("LORA RECEIVER");
//   // display.setCursor(100,0);
//   // display.println(sender,HEX);
//   display.setCursor(0,20);
//   display.print("Received packet:");
//   display.setCursor(0,30);
//   display.print(mensaje);
//   display.setCursor(0,40);
//   display.print("RSSI:");
//   display.setCursor(30,40);
//   display.print(rssi);
//   display.display();  
// }

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


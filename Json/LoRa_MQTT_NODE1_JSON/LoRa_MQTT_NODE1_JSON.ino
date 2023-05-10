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
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_BME280 bme; // I2C

const long frequency = 868.5E6;  // LoRa Frequency

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 12
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

const String batiment = "TMM(LTDS)";
const String salle = "343";
const char* sensorType = "BME280";

byte MasterNode = 0xFF;     
byte Node1 = 0xA1;
bool envoi = true;
int rssi;

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);

  bool status;
  status = bme.begin(0x77);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }  

  Serial.println("-- Default Test --");
 

  Serial.println();
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.setSyncWord(0xF3);

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Node");
  Serial.println("Only receive messages from gateways");
  Serial.println("Tx: invertIQ disable");
  Serial.println("Rx: invertIQ enable");
  Serial.println();

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
  display.print("LORA Node1 ");
  display.display();
  
  Serial.println("LoRa Node1");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  if (runEvery(250) && envoi==true) { // repeat every 1000 millis
  StaticJsonDocument<300> doc;
  doc["salle"] = salle;
  doc["sensorType"] = sensorType;
  doc["Temperature"] = round2(bme.readTemperature());
  doc["Humidity"] = round2(bme.readHumidity());
  doc["Pressure"] = round2(bme.readPressure()/100.0F);
  doc["RSSI"] = LoRa.packetRssi();
  char JSONmessageBuffer[150];          //variable dans laquelle sera stockée la version serialisée du json object 
  serializeJsonPretty(doc, JSONmessageBuffer);    //serialisation du Jsondocument

    String message = "  ";
    message += JSONmessageBuffer;
    LoRa_sendMessage(message,MasterNode,Node1); // send a message
    Display(message);
    envoi = false;
    Serial.println("Send Message!");
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message,byte MasterNode,byte OtherNode) {
  LoRa_txMode();                        // set tx mode
  LoRa.write(MasterNode);
  LoRa.write(OtherNode);
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
    // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address = Client1
  byte sender = LoRa.read();  // sender address = Master
  Serial.print(recipient);
  Serial.print(sender);

  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

    // if the recipient isn't this device or broadcast,
  if (recipient != Node1 || sender != MasterNode) {
    Serial.println("This message is not for me.");
    envoi = false;
    ;
    return;                             // skip rest of function
  }
  // else {
  //   LoRa_sendMessage(message);
  // }
  envoi = true;
  rssi = LoRa.packetRssi();
  Serial.print("Node Receive: ");
  Serial.println(message);
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

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

void Display(String argument){
  display.clearDisplay();
  display.setCursor(0,30);
  display.println(argument);
  display.display();  
}
double round2(double value){
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));
}

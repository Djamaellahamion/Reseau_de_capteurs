/*
  Ce programme est basé sur RadioLib SX127x Ping-Pong Example permet définir un HELTEC ESP32 LoRa comme un noeud qui transmet des données à une passerelle(Gateway).
Ce programme correspond au noeud Node6 connectée en i2c avec un capteur SHT31.
Il implémente la création d'un objet JSON contenant salle,adresseNode, un String correspondant au numéro du Node et les données du capteur SHT31.
Cet objet JSON est envoyé à la passerelle lorsque le TTGO LoRa ESP32 reçoit un message, de la part de la passerelle, contenant son adresse de noeud(0xA4).
*/

// include the library
#include <RadioLib.h>
#include <ArduinoJson.h>     //librairie JSON
#include <Wire.h>            // pour la communication i2c avec le capteur
#include "HT_SSD1306Wire.h"  //bibliothèque pour l'écran
#include "Adafruit_SHT31.h"  //librairie capteur SHT31
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

//configuration liaison I2C (le capteur n'est pas branché aux pins SDA et SCL par défaut)
#define SHT31_SDA 47
#define SHT31_SCL 48

struct Node{
  String numNode="Node5";
  byte addresseNode=0xA5;
};
Node Node;

bool wireStatus = Wire1.begin(SHT31_SDA, SHT31_SCL);  //configuration de la liaison i2C sur les pins 41 et 42

//driver permettant la connexion i2c avec le capteur en utilisant la nouvelle conguration avec SDA=pin41 et SCL=pin42
Adafruit_SHT31 sht31 = Adafruit_SHT31(&Wire1);
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst


//Variables ajoutées dans le JSON
String salle = "355";
String numNode = Node.numNode;
const char* sensorType = "SHT31";
float temperature;
float humidity;

//configuration pins LoRa

SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

int rssi;
// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;  // mettre à false pour un node
//fonction permettant d'arrondir:
double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));
}
// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // we sent or received  packet, set the flag
  operationDone = true;
}
void screendisplay(void){
  factory_display.drawString(0, 0, "Salle: ");
  factory_display.drawString(70, 0, salle);
  factory_display.drawString(0, 10, "SensorType: ");
  factory_display.drawString(70, 10, sensorType);
  factory_display.drawString(0, 20, "Temperature: ");
  factory_display.drawString(70, 20, String(temperature)+" °C");
  factory_display.drawString(0, 30, "Humidity: ");
  factory_display.drawString(70, 30, String(humidity)+ " %RH");
  factory_display.drawString(70, 50, "RSSI:"+ String(rssi)+ " dBm");
  factory_display.display();
  delay(100);
  factory_display.clear();
}

void setup() {
  Serial.begin(115200);

  //lancement et test de la communication i2c avec le capteur SHT31
  if (!sht31.begin(0x44)) {  // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 7, 7, 0x3895);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);
  factory_display.init();
	factory_display.clear();
	factory_display.display();

#if defined(INITIATING_NODE)
  // send the first packet on this node
  Serial.print(F("[SX1262] Sending first packet ... "));
  transmissionState = radio.startTransmit("I am a Node");
  transmitFlag = true;
#else
  // start listening for LoRa packets on this node
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
#endif
}

void loop() {
  temperature=round2(sht31.readTemperature());
  humidity=round2(sht31.readHumidity());
  screendisplay();
  // check if the previous operation finished
  if (operationDone) {
    // reset flag
    operationDone = false;

    if (transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        Serial.println(F("transmission finished!"));

      } else {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);
      }

      // listen for response
      radio.startReceive();
      transmitFlag = false;

    } else {
      // the previous operation was reception
      // print data and send another packet
      byte gateway[2];
      int state = radio.readData(gateway, 2);

      // wait a second before transmitting again
      //delay(250);

      // send another one
      if (gateway[1] == Node.addresseNode) {
        if (state == RADIOLIB_ERR_NONE) {
          // packet was successfully received
          Serial.println(F("[SX1262] Received packet!"));
          rssi=radio.getRSSI();
          // print data of the packet
          Serial.print(F("[SX1262] Data:\t\t"));
          Serial.println(String(gateway[1], HEX));

          // print RSSI (Received Signal Strength Indicator)
          Serial.print(F("[SX1262] RSSI:\t\t"));
          Serial.print(radio.getRSSI());
          Serial.println(F(" dBm"));

          // print SNR (Signal-to-Noise Ratio)
          Serial.print(F("[SX1262] SNR:\t\t"));
          Serial.print(radio.getSNR());
          Serial.println(F(" dB"));
        }
        Serial.print(F("[SX1262] Sending another packet ... "));
        //mis a jour des mesures du capteur
        // temperature=round2(sht31.readTemperature());
        // humidity=round2(sht31.readHumidity());
        //Création de l'objet JSON "doc"
        StaticJsonDocument<300> doc;
        //Ajout de variables dans le JSON
        doc["salle"] = salle;
        doc["sensorType"] = sensorType;
        doc["numNode"] = Node.numNode;
        doc["adresseNode"] =Node.addresseNode;
        doc["Temperature"] = round2(sht31.readTemperature());
        doc["Humidity"] = round2(sht31.readHumidity());
        //variable dans laquelle sera stockée la version serialisée du json object
        char JSONmessageBuffer[200];
        //serialisation du Jsondocument/objet pour l'envoie LoRa à la Gateway
        serializeJsonPretty(doc, JSONmessageBuffer);
        //transmission du JSON sérialisé
        transmissionState = radio.startTransmit(JSONmessageBuffer);
        Serial.println("Sending: ");
        Serial.println(JSONmessageBuffer);
        transmitFlag = true;
      }
    }
  }
  
}
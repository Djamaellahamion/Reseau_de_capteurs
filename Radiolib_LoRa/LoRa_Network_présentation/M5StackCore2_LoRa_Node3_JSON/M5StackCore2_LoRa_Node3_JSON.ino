/*
  Ce programme est basé sur RadioLib SX127x Ping-Pong Example permet définir un Heltech ESP32 LoRa comme un noeud qui transmet des données à une passerelle(Gateway).
Ce programme correspond au noeud Node6.
Il implémente la création d'un objet JSON contenant salle,adresseNode et un String correspondant au numéro du Node.
Cet objet JSON est envoyé à la passerelle lorsque le TTGO LoRa ESP32 reçoit un message, de la part de la passerelle, contenant son adresse de noeud(0xA5).
*/

// include the library
#include <RadioLib.h>
#include <ArduinoJson.h>  //librairie JSON

#include "Adafruit_SHT31.h"

#include <M5Core2.h>
#include "CUF_24px.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31(); // I2C

bool screen_state = false ;

#define LoRa_MOSI 23
#define LoRa_MISO 38
#define LoRa_SCK 18

#define LoRa_nss 27
#define LoRa_dio1 36 //pin d'interruption
#define LoRa_nrst 26
#define LoRa_busy 19
struct Node{
  String numNode="Node3";
  byte addresseNode=0xA3;
};
Node Node;
//Variables ajoutées dans le JSON

String salle = "353";
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

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // we sent or received  packet, set the flag
  operationDone = true;
}
double round2(double value){
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));

}

void capteur(){
  // check if the previous operation finished
  if(M5.Touch.ispressed()) {
    screen_state = !screen_state;
    delay(1000);
      if (screen_state){M5.Lcd.sleep();}
      else {M5.Lcd.wakeup();}
  }

  String salles = String("salle: ")+ salle;
  String sensor = "sensorType: SHT31";
  String temp = "Temperature: "+String(temperature) +" °C";
  String humid = "Humidite: " + String(humidity) + " %RH";
  M5.Lcd.drawString(salles, 160, 20,1);
  M5.Lcd.drawString(sensor, 160, 50,1);
  M5.Lcd.drawString(temp, 160, 80,1);
  M5.Lcd.drawString(humid, 160, 110,1);
  M5.Lcd.drawString("Rssi: "+String(rssi), 200, 190,1);
}

void setup() {
  M5.begin();  //Init M5Core2.
  M5.Lcd.setFreeFont(
      &unicode_24px);  //Set the GFX font to use. 
  M5.Lcd.setTextDatum(
      TC_DATUM);  //Set text alignment to center-up alignment.
  Serial.begin(115200);

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
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
  temperature = round2(sht31.readTemperature());
  humidity = round2(sht31.readHumidity());
  capteur();
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
        Serial.print(F("[SX1262] Sending another packet ... "));
        rssi=radio.getRSSI();
        temperature = round2(sht31.readTemperature());
        humidity = round2(sht31.readHumidity());
        //Création de l'objet JSON "doc"
        StaticJsonDocument<300> doc;
        //Ajout de variables dans le JSON
        doc["salle"] = salle;
        doc["numNode"] = Node.numNode;
        doc["adresseNode"] = Node.addresseNode;
        doc["sensorType"] = sensorType;
        doc["Temperature"] = round2(sht31.readTemperature());
        doc["Humidity"] = round2(sht31.readHumidity());
        //variable dans laquelle sera stockée la version serialisée du json object
        char JSONmessageBuffer[150];
        //serialisation du Jsondocument/objet pour l'envoie LoRa à la Gateway
        serializeJsonPretty(doc, JSONmessageBuffer);
        //transmission du JSON sérialisé
        transmissionState = radio.startTransmit(JSONmessageBuffer);

        transmitFlag = true;
        if (state == RADIOLIB_ERR_NONE) {
          // packet was successfully received
          Serial.println(F("[SX1262] Received packet!"));

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
      }
    }
  }
}




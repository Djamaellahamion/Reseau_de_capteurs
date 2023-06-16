/*
  Ce programme est basé sur RadioLib SX127x Ping-Pong Example permet définir un Heltech ESP32 LoRa comme un noeud qui transmet des données à une passerelle(Gateway).
Ce programme correspond au noeud Node5.
Il implémente la création d'un objet JSON contenant salle,adresseNode et un String correspondant au numéro du Node.
Cet objet JSON est envoyé à la passerelle lorsque le TTGO LoRa ESP32 reçoit un message, de la part de la passerelle, contenant son adresse de noeud(0xA5).
*/

// include the library
#include <RadioLib.h>
#include <ArduinoJson.h>  //librairie JSON
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

//Variables ajoutées dans le JSON
String salle = "353";
String numNode = "Node5";

//configuration pins LoRa
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

int const nbNodes = 6;
byte MasterNode = 0xFF;  //c'est moi
byte Node1 = 0xA1;
byte Node2 = 0xA2;
byte Node3 = 0xA3;
byte Node4 = 0xA4;
byte Node5 = 0xA5;
byte Node[nbNodes] = { MasterNode, Node1, Node2, Node3, Node4, Node5 };

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

void setup() {
  Serial.begin(115200);

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 7, 7, 0x12);
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
      if (gateway[1] == Node5) {
        Serial.print(F("[SX1262] Sending another packet ... "));

        //Création de l'objet JSON "doc"
        StaticJsonDocument<300> doc;
        //Ajout de variables dans le JSON
        doc["salle"] = salle;
        doc["numNode"] = numNode;
        doc["adresseNode"] = Node5;
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
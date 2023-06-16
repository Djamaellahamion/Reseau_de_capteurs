/*
Ce programme est basé sur RadioLib SX127x Ping-Pong Example permet définir un TTGO lilyGO ESP32 LoRa comme un noeud qui transmet des données à une passerelle(Gateway).
Ce programme correspond au noeud Node1 connectée en i2c avec un capteur GY63 MS5611.
Il implémente la création d'un objet JSON contenant salle,adresseNode, un String correspondant au numéro du Node et les données du capteur GY63.
Cet objet JSON est envoyé à la passerelle lorsque le TTGO LoRa ESP32 reçoit un message, de la part de la passerelle, contenant son adresse de noeud(0xA1).
*/

// include the library
#include <RadioLib.h>     //librarie LoRa
#include <ArduinoJson.h>  //librairie JSON
#include <Wire.h> // pour la communication i2c avec le capteur
//librairies capteur GY-BM(BMe280)
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  
#define LoRa_MOSI 27
#define LoRa_MISO 19
#define LoRa_SCK 5
#define LoRa_dio0 26

#define LoRa_nss 18
#define LoRa_dio1 33
#define LoRa_nrst 23

// BME280 GY-63 I2C(pins par défaut)
Adafruit_BME280 bme; // I2C

//Variables ajoutées dans le JSON
String salle = "353";
String numNode = "Node1";
const char* sensorType = "GY-BM(BME280)";

//fonction permettant d'arrondir:
double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));
}

//configuration pins LoRa
SX1276 radio = new Module(LoRa_nss, LoRa_dio0, LoRa_nrst, LoRa_dio1);

int const nbNodes = 4;
byte MasterNode = 0xFF;
byte Node1 = 0xA1;  //c'est moi
byte Node2 = 0xA2;
byte Node3 = 0xA3;
byte Node[nbNodes] = { MasterNode, Node1, Node2, Node3 };

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



  //test de la liaison i2c avec le capteur 
  bool status;
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 7, 7, 0x25);
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
  radio.setDio0Action(setFlag, RISING);

#if defined(INITIATING_NODE)
  // send the first packet on this node
  Serial.print(F("[SX1276] Sending first packet ... "));
  transmissionState = radio.startTransmit("I am a Node");
  transmitFlag = true;
#else
  // start listening for LoRa packets on this node
  Serial.print(F("[SX1276] Starting to listen ... "));
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
      if (gateway[1] == Node1) {
        if (state == RADIOLIB_ERR_NONE) {
          // packet was successfully received
          Serial.println(F("[SX1276] Received packet!"));

          // print data of the packet
          Serial.print(F("[SX1276] Data:\t\t"));
          Serial.println(String(gateway[1], HEX));

          // print RSSI (Received Signal Strength Indicator)
          Serial.print(F("[SX1276] RSSI:\t\t"));
          Serial.print(radio.getRSSI());
          Serial.println(F(" dBm"));

          // print SNR (Signal-to-Noise Ratio)
          Serial.print(F("[SX1276] SNR:\t\t"));
          Serial.print(radio.getSNR());
        }
        Serial.print(F("[SX1276] Sending another packet ... "));


        //Création de l'objet JSON "doc"
        StaticJsonDocument<300> doc;
        //Ajout de variables dans le JSON
        doc["salle"] = salle;
        doc["sensorType"] = sensorType;
        doc["numNode"]=numNode;
        doc["adresseNode"] = Node1;
        doc["Temperature"] = round2(bme.readTemperature());//valeur arrondie de la mesure de temperature
        doc["Pressure"] = round2(bme.readPressure()); //valeur arrondie de la mesure de pression
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
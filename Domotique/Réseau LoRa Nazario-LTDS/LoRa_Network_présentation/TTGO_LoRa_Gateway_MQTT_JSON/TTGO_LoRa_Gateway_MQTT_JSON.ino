/*
   Ce programme est basé sur RadioLib SX127x Ping-Pong Example et permet de définir un TTGO ESP32 LilyGo comme une Gateway qui envoie un message à plusieurs ESP32 LoRa (Nodes) les uns apres les autres
Et reçoit les messages de ces derniers et les publie en MQTT à un broker 
Le message envoyé depuis la Gateway contient l'octet adresse de la Gateway et l'octet adresse du Node de destinatination.
Le message reçu est un object JSON qui contient la salle, le numéro du Node ayant envoyé le message ainsi que son adresse.
Cet objet json est désérialisé afin d'extraire uniquement le numéro du node et ainsi pouvoir définir le topic sur lequel on envoie.
*/

// include the library
#include <RadioLib.h>      //librairie LoRa
#include <PubSubClient.h>  //librairie MQTT
#include <ArduinoJson.h>   //librairie Json
#include <WiFi.h>
#include <Adafruit_SSD1306.h>  //librairie ecran
//pins communication SPI de l'esp32 pour la communication avec la puce LoRa
#define LoRa_MOSI 27
#define LoRa_MISO 19
#define LoRa_SCK 5
#define LoRa_dio0 26  //pin d'interruption

#define LoRa_nss 18
#define LoRa_dio1 33  //pin d'interruption
#define LoRa_nrst 23

// uncomment the following only on one
// of the nodes to initiate the pings
#define INITIATING_NODE

//OLED pins
#define OLED_SDA 21  // Attention: Les broches SDA et SCL de l'affichage OLED sont inversées sur le dessin de LilyGO et sur GitHub.
#define OLED_SCL 22
#define OLED_RST 12       // Note: Le TTGO Lora32 v2 n'utilise pas le signal reset, mais la librairie Adafruit_SSD1306, oui.
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
//Display object for oled screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Connexion MQTT:
//-avec les indentifiant et mot de passe du poit d'accès wifi(et ceux du broker s'ils ont été définie)
//-l'@ IP du broker MQTT et le numéro de Port MQTT
const char* ssid = "freebox_1rueLinoVentura";
const char* password = "naza08laura69";
const char* mqttServer = "192.168.0.49";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";
String clienID = "LTDS";  //identifiant que l'on donne au moment de la connexion au broker

WiFiClient espClient;
PubSubClient client(espClient);

SX1276 radio = new Module(LoRa_nss, LoRa_dio0, LoRa_nrst, LoRa_dio1);

//octets définissant les adresses du master/gateway et des noeuds/nodes du réseau LoRa
byte MasterNode = 0xFF;  //c'est moi
byte Node1 = 0xA1;
byte Node2 = 0xA2;
byte Node3 = 0xA3;
byte Node4 = 0xA4;
byte Node5 = 0xA5;
byte Node6 = 0xA6;
byte Node7 = 0xA7;
byte Node[] = { MasterNode, Node1, Node2, Node3, Node4, Node5, Node6, Node7 };
int const nbNodes = sizeof(Node);

//La variable count défint l'indice du Node dans le tableau de Nodes. Ce qui nous est utile lors de la transmission
// A la première transmission, on envoie au Node d'indice count+1(soit 1)
int count = 0;

//variables pour définir un timer avec la fonction millis()
static unsigned long previousMillis = 0;
unsigned long currentMillis;

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;  // mettre à true pour le gateway


// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // we sent or received  packet, set the flag
  operationDone = true;
}
double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));
}

//la fonction suivante renvoie un booléen true lorsque le temps définit par le paramètre "timeout" est atteint.
//Elle nous permettra de définir un timeout au cours du quel si le Gateway n'a rien reçu il passe au noeud suivant
boolean runEvery(unsigned long timeout) {
  currentMillis = millis();
  if (currentMillis - previousMillis >= timeout) {
    previousMillis = currentMillis;
    transmitFlag = false;
    return true;
  }
  return false;
}


//la fonction suivante permet d'envoyer un message LoRa aux differents nodes dans l'ordre,
// elle permet de transmettre au noeud d'indice count+1 à chaque fois qu'elle est appelée
//Le message transmit est l'octet du Gateway et celui du noeud de destination
void send_message() {
  count++;
  count = count % nbNodes;
  if (count == 0) { count = 1; }
  byte message[] = { Node[0], Node[count] };
  //String message = "gateway"+String(Node[count],HEX);
  transmissionState = radio.startTransmit(message, 2);
  transmitFlag = true;
}


void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  //lancement de la connexion wifi
  WiFi.begin(ssid, password);
  //test de la connexion wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  //connexion wifi établi:
  Serial.println("Connected to the WiFi network");
  //lancement de la connexion au broker mqtt
  client.setServer(mqttServer, mqttPort);
  //test de la connexion au broker
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clienID.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  Serial.println(transmitFlag);

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  // carrier frequency:           868.0 MHz
  // bandwidth:                   125.0 kHz
  // spreading factor:            7
  // coding rate:                 7
  // sync word:                   0x34
  // output power:                10 dBm
  // preamble length:             20 symbols
  // amplifier gain:              1 (maximum gain)
  int state = radio.begin(868.0, 125.0, 7, 7, 0x3895,10);
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
  byte message[] = { Node[0], Node[count] };
  transmissionState = radio.startTransmit(message, 2);
  //transmissionState = radio.startTransmit("I am a Gateway");
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

  if (operationDone) {
    //reset du flag
    operationDone = false;

    //réintialisation du timer
    previousMillis = millis();

    if (transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        Serial.println(F("transmission finished!"));
        display.setCursor(0, 0);
        display.print("Transmission to Node" + String(count) + " succeed");
        display.display();
        display.clearDisplay();

      } else {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);
        display.setCursor(0, 0);
        display.print("Transmission to Node" + String(count) + " failed");
        display.display();
        display.clearDisplay();
      }

      // listen for response
      radio.startReceive();
      Serial.println("start receive");
      transmitFlag = false;
    } else {
      // the previous operation was reception
      // print data and send another packet
      String str;
      int state = radio.readData(str);
      Serial.print("read data");
      //test de l'abscence d'erreur
      if (state == RADIOLIB_ERR_NONE) {
        // packet was successfully received
        Serial.println(F("[SX1276] Received packet!"));
        display.setCursor(0, 0);
        display.print("Received message from Node" + String(count));
        


        // print data of the packet
        Serial.print(F("[SX1276] Data:\t\t"));
        Serial.println(str);

        // print RSSI (Received Signal Strength Indicator)
        Serial.print(F("[SX1276] RSSI:\t\t"));
        Serial.print(radio.getRSSI());
        Serial.println(F(" dBm"));
        display.setCursor(0, 50);  // on rajoute le rssi de la réception
        display.print("RSSI" + String(radio.getRSSI())+ "dBm");
        display.display();
        display.clearDisplay();

        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("[SX1276] SNR:\t\t"));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));
        //Création de l'objet JSON "doc" qui contiendra le message désérialisé
        StaticJsonDocument<300> doc;
        //Désérialisation du Json reçu
        DeserializationError error = deserializeJson(doc, str);
        // Test if parsing(deserialization) succeeds.
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        //extraction du numéro de Node
        String salle = doc["salle"].as<String>();  //méthode as<type>() permet de convertir le JsonVariant en type spécifié
        //création du topic MQTT
        String batiment = "TMM";

        String Topic = batiment + "/Salle/" + salle;
        //affichage du Topic
        Serial.println(Topic);
        //ajout du LoRa rssi et snr dans le JSON Objet reçu
        doc["Rssi"] = round2(radio.getRSSI());
        //variable dans laquelle sera stockée la version serialisée du json object
        char newstr[200];  //la taille du tableau de char est à modifié en fonction de ce qu'il y a dans le json(augmenter si nécessaire)


        //serialisation du Jsondocument/objet pour la publication MQTT
        serializeJsonPretty(doc, newstr);
        //publication mqtt et test de la publication:
        if (client.publish(Topic.c_str(), newstr) == true) {
          Serial.println("Success sending message");
          Serial.println(newstr);
          Serial.println(Topic);
        } else {
          Serial.println("Error sending message");
          ESP.restart();
        }
        client.loop();
        Serial.println("-------------");  //ligne de séparation
      }



      // wait a second before transmitting again
      delay(250);

      // send another one
      Serial.print(F("[SX1276] Sending another packet ... "));
      send_message();
    }
  } else {
    //lorsqu'aucune opération(réception ou transmission) n'a été faite au bout de 5 s on appelle la fontion send_message
    //pour envoyée au noeud suivant
    if (runEvery(500)) {  // on attend 1 seconde
      char indication[25];
      if (count == 0) {
        sprintf(indication, "Ceci est le noeud Master");
      } else {
        sprintf(indication, "Node%d abscent, je passe au Node suivant", count);
      }
      Serial.println(indication);
      display.print(indication);
      display.display();
      display.clearDisplay();
      send_message();
    }
  }
}

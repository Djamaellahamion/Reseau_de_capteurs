/*
   RadioLib SX127x Ping-Pong Example

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx127xrfm9x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/
int count = 1;
int boucle = 1;
unsigned long temps = 0;

// include the library
#include <RadioLib.h>
#define LoRa_MOSI 27
#define LoRa_MISO 19
#define LoRa_SCK 5
#define LoRa_dio0 26

#define LoRa_nss 18
#define LoRa_dio1 33
#define LoRa_nrst 23

// uncomment the following only on one
// of the nodes to initiate the pings
#define INITIATING_NODE

SX1276 radio = new Module(LoRa_nss, LoRa_dio0, LoRa_nrst,LoRa_dio1);

int const nbNodes = 6;
byte MasterNode = 0xFF;//c'est moi
byte Node1 = 0xA1;
byte Node2 = 0xA2;
byte Node3 = 0xA3;
byte Node4 = 0xA4;
byte Node5 = 0xA5;
byte Node[nbNodes] = { MasterNode, Node1,Node2,Node3,Node4,Node5};

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false; // mettre à true pour le gateway

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
  Serial.println(transmitFlag);

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.begin(868.0,125.0,7,7,0x12);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio0Action(setFlag, RISING);

  #if defined(INITIATING_NODE)
    // send the first packet on this node
    Serial.print(F("[SX1276] Sending first packet ... "));
    byte message[] = {Node[0],Node[count]};
    transmissionState = radio.startTransmit(message,2);
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
      while (true);
    }
  #endif
  temps = millis();
}

void loop() {
  // check if the previous operation finished
      //Serial.print(operationDone);
      // Serial.print("loop");
      // Serial.print("operationDone");Serial.print(operationDone);
      // Serial.print("transmitFlag");Serial.println(transmitFlag);


      //if((operationDone)||(runEvery(1001))) {
      if((operationDone)) {

        //if (operationDone==false) {transmitFlag = false;}
        // reset flag
        operationDone = false;
        //boucle=0;

        if(transmitFlag){
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
          Serial.print("start receive");
          transmitFlag = false;

        } else {
          // the previous operation was reception
          // print data and send another packet
          String str;
          int state = radio.readData(str);
          Serial.print("read data");

          if (state == RADIOLIB_ERR_NONE) {
            // packet was successfully received
            Serial.println(F("[SX1276] Received packet!"));

            // print data of the packet
            Serial.print(F("[SX1276] Data:\t\t"));
            Serial.println(str);

            // print RSSI (Received Signal Strength Indicator)
            Serial.print(F("[SX1276] RSSI:\t\t"));
            Serial.print(radio.getRSSI());
            Serial.println(F(" dBm"));

            // print SNR (Signal-to-Noise Ratio)
            Serial.print(F("[SX1276] SNR:\t\t"));
            Serial.print(radio.getSNR());
            Serial.println(F(" dB"));

          }

          // wait a second before transmitting again
          delay(1000);

          // send another one
          Serial.print(F("[SX1276] Sending another packet ... "));
          count++;
          count = count % nbNodes;
          if (count == 0){count=1;}
          byte message[] = {Node[0],Node[count]};
          //String message = "gateway"+String(Node[count],HEX);
          transmissionState = radio.startTransmit(message,2);
          transmitFlag = true;
          
        }
      }
      //else if (runEvery(1002)) {operationDone = true;}

    // temps = millis()-temps;
    // if (temps > 5000){temps = millis();}
    // Serial.println(temps);

    //if (runEvery(2671) == 1) {Serial.print("stop");}
    //Serial.println(runEvery(150));
    //Serial.println(temps);
    // boucle++;
    // //Serial.println(boucle);
    // if (boucle>1000000){boucle=0;operationDone = true;transmitFlag = false;}
    // //delay(100);
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
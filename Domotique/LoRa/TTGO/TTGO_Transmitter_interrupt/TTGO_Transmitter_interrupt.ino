/*
   RadioLib SX126x Transmit with Interrupts Example

   This example transmits LoRa packets with one second delays
   between them. Each packet contains up to 256 bytes
   of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Other modules from SX126x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>

//from https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/ArduinoLoRa/LoRaReceiver/utilities.h
// #define RADIO_SCLK_PIN              5
// #define RADIO_MISO_PIN              19
// #define RADIO_MOSI_PIN              27
// #define RADIO_CS_PIN                18
// #define RADIO_DIO0_PIN               26
// #define RADIO_RST_PIN               23
// #define RADIO_DIO1_PIN              33
// #define RADIO_BUSY_PIN              32


#define LoRa_MOSI 27
#define LoRa_MISO 19
#define LoRa_SCK 5
#define LoRa_dio0 26

#define LoRa_nss 18
#define LoRa_dio1 33
#define LoRa_nrst 23

SX1276 radio = new Module(LoRa_nss, LoRa_dio0, LoRa_nrst,LoRa_dio1);

int const nbNodes = 2;
byte MasterNode = 0xFF;//c'est moi
byte Node1 = 0xA1;
byte Nodes[nbNodes] = { MasterNode, Node1};

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

void setup() {
  Serial.begin(115200);

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
  // when packet transmission is finished
  radio.setDio0Action(setFlag,RISING);

  // start transmitting the first packet
  Serial.print(F("[SX1276] Sending first packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long
  transmissionState = radio.startTransmit("Ola que tal!");

  // you can also transmit byte array up to 256 bytes long
  /*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                      0x89, 0xAB, 0xCD, 0xEF};
    state = radio.startTransmit(byteArr, 8);
  */
}

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;
}

void loop() {
  // check if the previous transmission finished
  if(transmittedFlag) {
    // reset flag
    transmittedFlag = false;

    if (transmissionState == RADIOLIB_ERR_NONE) {
      // packet was successfully sent
      Serial.println(F("transmission finished!"));

      // NOTE: when using interrupt-driven transmit method,
      //       it is not possible to automatically measure
      //       transmission data rate using getDataRate()

    } else {
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);

    }

    // clean up after transmission is finished
    // this will ensure transmitter is disabled,
    // RF switch is powered down etc.
    radio.finishTransmit();

    // wait a second before transmitting again
    delay(1000);

    // send another one
    Serial.print(F("[SX1276] Sending another packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    String str = String(Nodes[0],HEX)+","+String(Nodes[1],HEX)+","+"Ola que tal!";
    transmissionState = radio.startTransmit(str);
    //int state = radio.startTransmit(Nodes, 2);

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      int state = radio.startTransmit(byteArr, 8);
    */
  }
}
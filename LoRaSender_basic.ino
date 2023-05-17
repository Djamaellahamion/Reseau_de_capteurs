#include <SPI.h>
#include <LoRa.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 12
#define DIO0 26

int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

    //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  LoRa.setSyncWord(0xF3);
  LoRa.setFrequency(868E6);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(1);

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("Bonjour ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(2000);
}

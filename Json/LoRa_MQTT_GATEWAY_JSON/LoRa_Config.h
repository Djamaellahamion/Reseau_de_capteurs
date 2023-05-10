//Configuration des pins et des paramètres physiques LoRa
#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DIO0    26
#define BAND    868.5E6        //Bande de fréquence, canal
#define SF                    //spreading factor
#define SBW                  //Bande passante
#define SW          0xF3    //SyncWord
#define CR                  //CodingRate
#define MASTERNODE  0xFF
#define NODE1       0xA1
#define NODE2       0xA2
#define NODE3       0xA3
#define NODE4       0xA4
#define NODE5       0xA5
#define NB_NODES    6
byte NODES[NB_NODES]={
  MASTERNODE, NODE1, NODE2, NODE3, NODE4, NODE5
};


void set_LoRa() {
  char buff[128];
  SPI.begin(SCK, MISO, MOSI);
  LoRa.setPins(SS, RST, DIO0);
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  if (!LoRa.begin(BAND)) {
    Serial.println("Démarrage LoRa échoué. Vérifier les connexions.");
    while (true)
      ;  // si la communication échoue, on ne fait rien
  }
  //sprintf(buff, "BAND=%f, SF=%d, SBW=%f, SW=%X, CR=%d\n", BAND, SF, SBW, SW, CR);
  //Serial.println("buff");
  //LoRa.setSpreadingFactor(SF);
  //LoRa.setSignalBandwidth(SBW);
  LoRa.setSyncWord(SW);
  //LoRa.setCodingRate4(CR);
}
void set_LoRa_Pins(int sck, int miso, int mosi, int ss, int rst, int dio0, unsigned long freq, unsigned sbw, int sf, uint8_t sw) {
  SPI.begin(sck, miso, mosi);
  LoRa.setPins(ss, rst, dio0);
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  if (!LoRa.begin(freq)) {
    Serial.println("Démarrage LoRa échoué. Vérifier les connexions.");
    while (true)
      ;  // si la communication échoue, on ne fait rien
  }
  //sprintf(buff, "BAND=%f, SF=%d, SBW=%f, SW=%X, CR=%d\n", freq, sf, sbw, sw, cr);
  //Serial.println("buff");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(sw);
}

//Terminal mode IQ
#ifdef TERMINAL
void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

#endif

//Gateway mode IQ
#ifdef GATEWAY
void LoRa_rxMode() {
  LoRa.disableInvertIQ();  // normal mode
  LoRa.receive();          // set receive mode
}

void LoRa_txMode() {
  LoRa.idle();            // set standby mode
  LoRa.enableInvertIQ();  // active invert I and Q signals
}
void LoRa_sendMessage(String message, byte MasterNode, byte otherNode) {
  LoRa_txMode();           // set tx mode
  LoRa.beginPacket();      // start packet
  LoRa.write(otherNode);   // add destination address
  LoRa.write(MasterNode);  // add sender address
  LoRa.print(message);     // add payload
  LoRa.endPacket(true);    // finish packet and send it
}

#endif

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

#define IDREQ_MQP 0x31   //requete ID pour publication MQTT
#define IDREQ_MQS 0x41   //requete ID pour abonnement MQTT
#define IDACK_MQP 0x32   //reponse ACK pour publication MQTT
#define IDACK_MQS 0x42   //reponse ACK pour abonnement MQTT
#define DTPUB     0x33
#define DTPUBACK  0x34
RTC_DATA_ATTR uint32_t termID, gwID;
RTC_DATA_ATTR int stage1_flag=0, cycle_count=0;// flag de controle des paquets IDREQ et IDACK
int stage2_flag=0;

#if def TERMINAL
RTC_DATA_ATTR char CKEY[17]; 
#endif

#include "mbedtls/aes.h"
void encrypt(unsigned char*plainText, char *key, unsigned char *outputBuuffer, int nblocks){
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_xts_setkey_enc(&aes, (const unsigned char*)key, strlen(key)*8);
  for(int i=0; i<nblocks; i+)
  {
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT,(const unsigned char*)(chipherText+i16), outputBuffer+i*16);
  }
  mbedtls_aes_free(&aes);
}
typedef union
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;   //destination identifier chipID
    uint32_t sid;   //source identifier chipID
    uint8_t con[2];
    char pass[16];
    int tout; //timeout
    uint8_t pad[2];
  }pack;
}conframe_t; // trames IDACK et IDREQ

typedef union{
  uint8_t frame[64]; 
  struct
  {
    uint32_t did;   //destination identifier chipID
    uint32_t sid;   //source identifier chipID
    uint8_t con[2];
    int channel;
    char keyword[16];
    float sens[8];
    uint16_t tout;
  }pack;
}Tsframe_t;

typedef union{
  uint8_t frame[64]; 
  struct
  {
    uint32_t did;   //destination identifier chipID
    uint32_t sid;   //source identifier chipID
    uint8_t con[2];
    char topic[48];
    cahr mess[48];
    int tout;
    uint8_t pad[2];
  }pack;
}MQTTframe_t; // trame MQTT

#ifdef TERMINAL

void send_IREQ(char *pass){
  conframe_t scf,sccf;
  LoRa_txMode();
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)0;
  scf.pack.did=(uint32_t)termID;
  scf.pack.con[0]=(uint8_t)(MODE*16+1);
  scf.pack.con[1]=0x00;
  if(pass!=NULL)
  strncpy(scf.pack.pass,pass,16);
  LoRa.write(scf.frame,32);
  LoRa.endPacket(true);
}

void send_DTPUB(char *topic, char *mess){
  MQTTframe_t sdf,sdcf;
  LoRa_txMode();
  LoRa.beginPacket();
  sdf.pack.did=(uin32_t)gwID;
  sdf.pack.did=(uin32_t)termID;
  sdf.pack.con[0]=(uint8_t)(MODE*16+3);
  scf.pack.con[1]=0x00;
  strcpy(sdf.pack.topic, topic);
  strcpy(sdf.pack.mess,mess);
  sdf.pack.tout=0;
  encrypt(sdf.frame, CKEY, sdcf.frame,7);
  LoRa.write(sdcf.frame, 112);
  LoRa.endPacket(true);
}
#endif


#ifdef GATEWAY

void send_IDACK(char *aes, uint16){
  conframe_t scf,sccf;
  LoRa_txMode();
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)termID;
  scf.pack.sid=(uint32_t)gwID;
  scf.pack.con[0]=(uint8_t)MODE*16+2;
  scf.pack.con[1]=0x00;
  strncpy(scf.pack.pass,aes,16);
  scf.pack.tout=tout;
  LoRa.write(scf.frame,32);
  LoRa.endPacket(true);
}


void send_DTPUBACK(char *topic, char *mess, int tout){
  MQTTframe_t sdf,sdcf;
  LoRa_txMode();
  LoRa.beginPacket();
  sdf.pack.did=(uin32_t)termID;
  sdf.pack.did=(uin32_t)gwID;
  sdf.pack.con[0]=(uint8_t)(MODE*16+4);
  scf.pack.con[1]=0x00;
  strcpy(sdf.pack.topic, topic, 48);
  strcpy(sdf.pack.mess,mess, 48);
  sdf.pack.tout=tout;
  encrypt(sdf.frame, CKEY, sdcf.frame,7);
  LoRa.write(sdcf.frame, 112);
  LoRa.endPacket(true);
}
#endif

void onReceive(int packetSize)
{
  if(packetSize==32)
  {
    conframe_t rcf;
    int i=0;
    while(LoRa.available()){
      rcf.frame[i]=LoRa.read();
      i++;
    }
    xQueueReset(con_queue);
    xQueueSendFromISR(con_queue, rec.frame, NULL);
    Serial.println("Received IDREQ_MODE");
  }
  if(packetSize==112){
    MQTTframe_t rdf,rdcf;
    int i=0;
    char ckey[17];
    strncpy(ckey, CKEY, 16);
    ckey[16]='\0';
    while(LoRa.available()){
      rcf.frame[i]=LoRa.read();
      i++;
    }
  }
}
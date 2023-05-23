#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#include <M5Core2.h>
#include "CUF_24px.h"
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

const char* ssid = "ESP32_SFA";
const char* password =  "Rose1981";
const char* mqttServer = "156.18.46.123";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";

const String batiment = "TMM";
const String salle = "TMM044";
const String Post = batiment + "/"+ salle; // c'est le post publié sur le broker
const char* sensorType = "BME680";
 
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  M5.begin();  //Init M5Core2.
  M5.Lcd.setFreeFont(
      &unicode_24px);  //Set the GFX font to use. 
  M5.Lcd.setTextDatum(
      TC_DATUM);  //Set text alignment to center-up alignment.
  Serial.begin(115200);
  Serial.println(F("BME680 async test"));

  Serial.println(Post);

  // Initialize MS5611 sensor 
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
 
    // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(salle.c_str(), mqttUser, mqttPassword )) {
        
      Serial.println("connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
}

void loop() {
  
    // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());



  StaticJsonDocument<300> doc;

  float rssi = WiFi.RSSI();

  //long realPressure = ms5611.readPressure();
  //double Pressure = realPressure/100;
 
  doc["salle"] = salle;
  doc["sensorType"] = sensorType;
  doc["Temperature"] = round2(bme.temperature);
  doc["Humidity"] = round2(bme.humidity);
  doc["Pressure"] = round2(bme.pressure / 100.0);
  doc["VOC"] = round2(bme.gas_resistance / 1000.0);
  doc["RSSI"] = rssi;


  char JSONmessageBuffer[200];
  serializeJsonPretty(doc, JSONmessageBuffer);
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
  M5.Lcd.fillScreen(0); // equivalent à clear screen

  if (client.publish(Post.c_str(), JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
    M5.Lcd.drawString("Success sending message", 160, 200,1);
  } else {
    Serial.println("Error sending message");
    M5.Lcd.drawString("Error sending message", 160, 200,1);
  }
 
  client.loop();
  Serial.println("-------------");
  // gestion de l'affichage
  
  String salle = "salle: TMM044";
  String sensor = "sensorType: BME680";
  String temp = String("Temperature: ")+(round2(bme.temperature))+String(" °C");
  String humid = String("Humidite: ") + (round2(bme.humidity)) + String(" %RH");
  String pres = String("Pression: ") + (round2(bme.pressure / 100.0)) + " hPa";
  String voc = String("qualité Air: ") + round2(bme.gas_resistance / 1000.0) ;
  M5.Lcd.drawString(salle, 160, 20,1);
  M5.Lcd.drawString(sensor, 160, 50,1);
  M5.Lcd.drawString(temp, 160, 80,1);
  M5.Lcd.drawString(humid, 160, 110,1);
  M5.Lcd.drawString(pres, 160, 140,1);
  M5.Lcd.drawString(voc, 160, 170,1);

  delay(5000);
 
}

double round2(double value){
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));

}
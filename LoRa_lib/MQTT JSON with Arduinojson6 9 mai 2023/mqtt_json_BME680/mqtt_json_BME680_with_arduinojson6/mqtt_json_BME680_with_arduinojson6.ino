#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
 
const char* ssid = "ESP32_SFA";
const char* password =  "Rose1981";
const char* mqttServer = "156.18.46.123";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";
//contenu du json object
const String batiment = "D5_bis";
const String salle = "353";
const String Topic = batiment + "/"+ salle; // c'est le topic sur lequel on publie sur le broker
const char* sensorType = "BME280";
 
WiFiClient espClient;
PubSubClient client(espClient);

// BME280 I2C
Adafruit_BME280 bme;
 
void setup() {
 
  Serial.begin(115200);
  Serial.println();

//Initialize BME280 sensor 
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
 
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
 
  StaticJsonDocument<300> doc;
  float rssi = WiFi.RSSI();
  doc["salle"] = salle;
  doc["sensorType"] = sensorType;
  doc["Temperature"] = round2(bme.readTemperature());
  doc["Humidity"] = round2(bme.readHumidity());
  doc["Pressure"] = round2(bme.readPressure()/100.0F);
  doc["RSSI"] = rssi;

//JsonArray values = doc.createNestedArray("values");
  

  // values.add(bme.readTemperature());
  //values.add(round2(bme.readTemperature()));
  // values.add(21);
  // values.add(23);
  char JSONmessageBuffer[150];          //variable dans laquelle sera stockée la version serialisée du json object 
  serializeJsonPretty(doc, JSONmessageBuffer);    //serialisation du Jsondocument
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
  if (client.publish(Post.c_str(), JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
 
  client.loop();
  Serial.println("-------------");
 
  delay(5000);
 
}

double round2(double value){
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));

}
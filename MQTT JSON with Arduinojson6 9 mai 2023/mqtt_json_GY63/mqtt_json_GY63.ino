#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MS5611.h>
 
const char* ssid = "ESP32_SFA";
const char* password =  "Rose1981";
const char* mqttServer = "156.18.46.123";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";

const String batiment = "TMM";
const String salle = "TMM355";
const String Post = batiment + "/"+ salle; // c'est le post publié sur le broker
const char* sensorType = "MS5611";
 
WiFiClient espClient;
PubSubClient client(espClient);

// MS5611 GY-63 I2C
MS5611 ms5611;
 
void setup() {
 
  Serial.begin(115200);
  Serial.println();

  Serial.println(Post);

  // Initialize MS5611 sensor 
  if (!ms5611.begin()) {
    Serial.println("Could not find a valid MS5611 sensor, check wiring!");
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

  long realPressure = ms5611.readPressure();
  //double Pressure = realPressure/100;
 
  doc["salle"] = salle;
  doc["sensorType"] = sensorType;
  doc["Temperature"] = round2(ms5611.readTemperature());
  
  
  //Serial.print(realPressure); Serial.print(Pressure);
  doc["Pressure"] = realPressure;
  doc["RSSI"] = rssi;

  // JsonArray& values = doc.createNestedArray("values");
  

  // values.add(bme.readTemperature());
 
  //values.add(round2(bme.readTemperature()));
  // values.add(21);
  // values.add(23);
 
  char JSONmessageBuffer[150];
  serializeJsonPretty(doc, JSONmessageBuffer);
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
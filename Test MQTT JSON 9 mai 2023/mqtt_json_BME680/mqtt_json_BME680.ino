#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
 
const char* ssid = "ESP32_SFA";
const char* password =  "Rose1981";
const char* mqttServer = "156.18.46.123";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";

const String batiment = "TMM";
const String salle = "TMM353";
const String Post = batiment + "/"+ salle; // c'est le post publié sur le broker
const char* sensorType = "BME680";
 
WiFiClient espClient;
PubSubClient client(espClient);

// BME280 I2C
Adafruit_BME680 bme;
 
void setup() {
 
  Serial.begin(115200);
  Serial.println();

  // Initialize BME280 sensor 
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
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
 
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();

  float rssi = WiFi.RSSI();
 
  JSONencoder["salle"] = salle;
  JSONencoder["sensorType"] = sensorType;
  JSONencoder["Temperature"] = round2(bme.readTemperature());
  JSONencoder["Humidity"] = round2(bme.readHumidity());
  JSONencoder["Pressure"] = round2(bme.readPressure()/100.0F);
  JSONencoder["RSSI"] = rssi;

  // JsonArray& values = JSONencoder.createNestedArray("values");
  

  // values.add(bme.readTemperature());
 
  //values.add(round2(bme.readTemperature()));
  // values.add(21);
  // values.add(23);
 
  char JSONmessageBuffer[150];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
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
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "Adafruit_SHT31.h"

#define SHT31_SDA 41
#define SHT31_SCL 42

bool wireStatus = Wire1.begin(SHT31_SDA, SHT31_SCL);

bool enableHeater = false;
uint8_t loopCnt = 0;

const char* ssid = "freebox_1rueLinoVentura";
const char* password =  "naza08laura69";
const char* mqttServer = "192.168.0.57";
const int mqttPort = 1883;
const char* mqttUser = "pi_broker";
const char* mqttPassword = "Rose1981";
//contenu du json object
const String batiment = "1rueLinoVentura";
const String salle = "Salle_Manger";
const String Topic = batiment + "/"+ salle; // c'est le topic sur lequel on publie sur le broker
const char* sensorType = "SHT31";
 
WiFiClient espClient;
PubSubClient client(espClient);

// SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31( &Wire1);

//SSD1306
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

void WIFISetUp(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.begin(ssid,password);//fill in "Your WiFi SSID","Your Password"
	delay(100);

	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
		factory_display.drawString(0, 0, "Connecting...");
		factory_display.display();
	}

	factory_display.clear();
	if(WiFi.status() == WL_CONNECTED)
	{
		factory_display.drawString(0, 0, "Connecting...OK.");
		factory_display.display();
//		delay(500);
	}
	else
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "Connecting...Failed");
		factory_display.display();
		//while(1);
	}
	factory_display.drawString(0, 30, "WIFI Setup done");
	factory_display.display();
	delay(500);
}

void setup() {
 
  Serial.begin(115200);
  Serial.println();
// Initialize the SSD1306 using the HT_SSD1306Wire.h
  factory_display.setFont(ArialMT_Plain_16);
  factory_display.init();
	factory_display.clear();
	factory_display.display();

//Initialize BME280 sensor 
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");


// WiFi setup
  WIFISetUp();
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
  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  StaticJsonDocument<300> doc;
  float rssi = WiFi.RSSI();
  doc["salle"] = salle;
  doc["sensorType"] = sensorType;
  doc["Temperature"] = round2(t);
  doc["Humidity"] = round2(h);
  doc["RSSI"] = rssi;



  char JSONmessageBuffer[150];          //variable dans laquelle sera stockée la version serialisée du json object 
  serializeJsonPretty(doc, JSONmessageBuffer);    //serialisation du Jsondocument
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
  if (client.publish(Topic.c_str(), JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
 
  client.loop();
  Serial.println("-------------");

  factory_display.clear();
  factory_display.setFont(ArialMT_Plain_16);
  factory_display.drawString(0, 10, "Temp= ");
  factory_display.drawString(60, 10, String(t));
  factory_display.drawString(0, 40, "Humid= ");
  factory_display.drawString(60, 40, String(h));

  factory_display.setFont(ArialMT_Plain_10);
  factory_display.drawString(105, 15, "°C");
  factory_display.drawString(105, 45, "%RH");
  factory_display.display();
  delay(100);

 
  delay(5000);
 
}

double round2(double value){
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));

}
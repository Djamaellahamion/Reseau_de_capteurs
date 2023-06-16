double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
  // return ((round(number*(10^precision)))/(10^precision));
}

#ifdef SHT31

#define SHT31_SDA 47
#define SHT31_SCL 48

bool wireStatus = Wire1.begin(SHT31_SDA, SHT31_SCL);  //configuration de la liaison i2C sur les pins 41 et 42

//driver permettant la connexion i2c avec le capteur en utilisant la nouvelle conguration avec SDA=pin41 et SCL=pin42
Adafruit_SHT31 sht31 = Adafruit_SHT31(&Wire1);

struct Sensor{
const char* sensorType = "SHT31";
float temperatur;
float humidite;
float mesures[]={&temperature, &humidite};

}
Sensor Sensor;
 void sht31_initialise(){
if (!sht31.begin(0x44)) {  // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void mesure(void){
for(int i=0; i< sizeof(Sensor.mesures);i++){
Sensor.mesure[0]=round2(sht31.readTemperature());
Sensor.mesure[1]=round2(sht31.readHumidity());
}























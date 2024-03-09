#include <ESP8266WiFi.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/*//how to connect Nodemcu to GPS module
          ||
          ||
          \/
//nodemcu GPIO 12=D6 to Gps Tx pin Green wire and nodemcu GPIO 13=D7 to Gps Rx pin White Wire*/

static const int RXPin = 13, TXPin = 12;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial softSerial(RXPin, TXPin);

// WiFi parameters
char ssid[] = "mine";  // Enter your Wifi Username
char pass[] = "12345678";  // Enter your Wifi password

// Adafruit IO
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883

//Enter the username and key from the Adafruit IO etwahirwa
//#define AIO_USERNAME    "etwahirwa"//
//#define AIO_KEY         "aio_ovVs31n7aoW284dAyIacNs5wjMFb"

//Enter the username and key from the Adafruit IO Evariste
#define AIO_USERNAME "Genius_Boy"
#define AIO_KEY "aio_rwZZ92BFSely6bm86dk2WYA5Xfsn"

WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish GPSLocation = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/location/csv");
// Setup feeds for temperature & humidity
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

Adafruit_MQTT_Publish c02 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/C02");
Adafruit_MQTT_Publish c0 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/C0");
Adafruit_MQTT_Publish nh3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/NH3");

#include "DHT.h"

//#define DHTPIN 2
#define DHTPIN 2

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include <MQUnifiedsensor.h>
//Definitions
#define placa "ESP8266"
#define Voltage_Resolution 5
#define pin A0                  //Analog input 0 of your arduino
#define type "MQ-135"           //MQ135
#define ADC_Bit_Resolution 10   // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6  //RS / R0 = 3.6 ppm
//#define calibration_button 13 //Pin to calibrate your sensor
//#define LED_PIN 16    // LED pin

//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

#define ADC_RESOLUTION 1024         // ADC resolution of the NodeMCU (10-bit ADC)
#define VOLTAGE_DIVIDER_R1 10000.0  // Resistance of the resistor connected to sensor (in ohms)
#define VOLTAGE_DIVIDER_R2 10000.0  // Resistance of the resistor connected to sensor (in ohms)
#define CO2_ZERO_VOLTAGE 1.4        // Output voltage of MQ135 sensor in clean air
#define CO2_PPM_PER_VOLTAGE 100     // PPM per unit of voltage change (adjust as per sensor specifications)


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);


float speed_mph = 0;
float alltitude = 0;
float lati;   //Storing the Latitude
float longi;  //Storing the Longitude

char gpsdata[120];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.begin();
  dht.begin();
  softSerial.begin(GPSBaud);
  //pinMode(led,OUTPUT);
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // connect to adafruit io
  connect();

  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1);  //_PPM =  a*ratio^b

  /**********  MQ Init ***************/
  //Remarks: Configure the pin of arduino as input.
  /****************************/
  MQ135.init();

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();  // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
    while (1)
      ;
  }
}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if (ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqtt.ping(3)) {
    // reconnect to adafruit io
    if (!mqtt.connected())
      connect();
  }

  getCoordinates();

  Serial.print("Lati = ");
  Serial.print(lati, 6);
  Serial.print("\tLongi = ");
  Serial.println(longi, 6);

  if (!GPSLocation.publish(gpsdata)) {  //Publish to Adafruit
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }


  // Grab the current state of the sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  int humidity_data = (int)hum;
  int temperature_data = (int)temp;

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(humidity_data);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature_data);
  Serial.println("  C");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperature_data);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humid:");
  lcd.print(humidity_data);
  lcd.print(" %");
  delay(1500);
  MQ135.update();  // Update data, the arduino will be read the voltage on the analog pin

  MQ135.setA(605.18);
  MQ135.setB(-3.937);             // Configurate the ecuation values to get CO concentration
  float CO = MQ135.readSensor();  // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  int val;
  val = (int)CO;
  Serial.print("CO:");
  Serial.println(val);

  MQ135.setA(110.47);
  MQ135.setB(-2.862);              // Configurate the ecuation values to get CO2 concentration
  float CO2 = MQ135.readSensor();  // Sensor will read PPM concentration using

  int sensorValue = analogRead(pin);                          // Read analog voltage from MQ135 sensor
  float voltage = sensorValue * (5.0 / ADC_RESOLUTION);       // Convert analog reading to voltage (5.0V reference voltage)
  float voltageRatio = voltage / CO2_ZERO_VOLTAGE;            // Calculate ratio with clean air voltage
  float CO2ppm = (voltageRatio - 1.0) * CO2_PPM_PER_VOLTAGE;  // Convert voltage ratio to CO2 concentration in ppm


  int val1;
  val1 = (int)(CO2ppm * -7);
  Serial.print("CO2: ");
  Serial.println(val1);

  MQ135.setA(102.2);
  MQ135.setB(-2.473);  // Configurate the ecuation values to get NH4 concentration
  float NH3 = MQ135.readSensor();
  int val2;
  val2 = (int)NH3;

  Serial.print("NH3: ");
  Serial.println(val2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO:");
  lcd.print(val);

  lcd.setCursor(8, 0);
  lcd.print("CO2:");
  lcd.print(val1);

  lcd.setCursor(0, 1);
  lcd.print("NH3:");
  lcd.print(val2);
  //delay(1500);


  // Publish data
  if (!temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));

  if (!humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  if (!c02.publish(val1))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  if (!c0.publish(val))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  if (!nh3.publish(val2))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));


  // Repeat every 10 seconds
  delay(10000);
}



void getCoordinates() {
  readGPSData();
  char *p = gpsdata;
  // add speed value
  dtostrf(speed_mph, 2, 6, p);
  p += strlen(p);
  p[0] = ',';
  p++;

  // concat latitude
  dtostrf(lati, 2, 6, p);
  p += strlen(p);
  p[0] = ',';
  p++;

  // concat longitude
  dtostrf(longi, 3, 6, p);
  p += strlen(p);
  p[0] = ',';
  p++;

  // concat altitude
  dtostrf(alltitude, 2, 6, p);
  p += strlen(p);

  // null terminate
  p[0] = 0;
}

void readGPSData() {
  lcd.clear();
  if (gps.location.isValid()) {
    lati = gps.location.lat();
    longi = gps.location.lng();
    Serial.print("Lati: ");
    Serial.print(lati, 6);
    Serial.print("\tLongi: ");
    Serial.println(longi, 6);
    // Construct Google Maps URL
    Serial.print("Google Maps URL: ");
    Serial.print("https://www.google.com/maps/place/");
    Serial.print(lati, 6);
    Serial.print(",");
    Serial.println(longi, 6);

    lcd.setCursor(0, 0);
    lcd.print("Lat: ");
    lcd.print(gps.location.lat(), 6);
    lcd.print(" ");
    lcd.print(gps.location.lat() > 0 ? "N" : "S");

    lcd.setCursor(0, 1);
    lcd.print("Lon: ");
    lcd.print(gps.location.lng(), 6);
    lcd.print(" ");
    lcd.print(gps.location.lng() > 0 ? "E" : "W");
    delay(1500);
  }
  waitGPS(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println("Waiting for data...");
}

static void waitGPS(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (softSerial.available())
      gps.encode(softSerial.read());
  } while (millis() - start < ms);
}

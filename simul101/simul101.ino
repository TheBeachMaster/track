#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>

char ssid[] = "AFRONANA";     
char pass[] = "Nanafam4"; 
int status = WL_IDLE_STATUS;

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "ArthurKen"
#define AIO_KEY         "c3fc507024c24cb19935eb9b896d11b8"

/************ Global State (you don't need to change this!) ******************/

//Set up the wifi client
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

#define halt(s) { Serial.println(F( s )); while(1);  }

Adafruit_MQTT_Publish readings = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/readings");
Adafruit_MQTT_Publish location_lat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lat");
Adafruit_MQTT_Publish location_long = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/long");
int32_t longitude,latitude,vibrationPinValue;


#define LEDPIN 13


void setup() {
  WiFi.setPins(9, 3, 4);

  while (!Serial);
  Serial.begin(115200);
  // Initialise the Client
  Serial.print(F("\nInit the WiFi module..."));
  // check for the presence of the breakout
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WINC1500 not present");
    // don't continue:
    while (true);
  }
  Serial.println("ATWINC OK!");
  pinMode(LEDPIN, OUTPUT);
  
  connectToWPA();
}


void loop() {
    
randomSeed(analogRead(0));
MQTT_connect();
vibrationPinValue = random(150, 800);  
longitude = random(32,42);
latitude = random(35);
processReadings(vibrationPinValue, longitude, latitude);
}
void processReadings(int32_t value, int32_t lon, int32_t lat)
{
    char values[50] ;
	snprintf(values,16,"%lu",value);
     Serial.println(values);
	 Serial.print(F("Longitiude: "));
	 Serial.println(lon);
	 Serial.print(F("Latitude: "));
	 Serial.println(lat);
	readings.publish(value);
	location_long.publish(lon);
	location_lat.publish(lat);
    if(value >= 500)
    {
        Serial.println("CRITICAL VALUES");
	} 
	delay(5000);
}
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
void connectToWPA()
{
	Serial.println(F("Setting up connection...."));
	// check for the presence of the shield:
	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		// don't continue:
		while (true);
	}
	// attempt to connect to Wifi network:
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to WPA SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		delay(10000);
	}
	// you're connected now, so print out the data:
	Serial.print("You're connected to the network");
	printCurrentNet();
	printWifiData();
}


void printWifiData() {
	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
	Serial.println(ip);

	// print your MAC address:
	byte mac[6];
	WiFi.macAddress(mac);
	Serial.print("MAC address: ");
	Serial.print(mac[5], HEX);
	Serial.print(":");
	Serial.print(mac[4], HEX);
	Serial.print(":");
	Serial.print(mac[3], HEX);
	Serial.print(":");
	Serial.print(mac[2], HEX);
	Serial.print(":");
	Serial.print(mac[1], HEX);
	Serial.print(":");
	Serial.println(mac[0], HEX);

}

void printCurrentNet() {
	// print the SSID of the network you're attached to:
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	// print the MAC address of the router you're attached to:
	byte bssid[6];
	WiFi.BSSID(bssid);
	Serial.print("BSSID: ");
	Serial.print(bssid[5], HEX);
	Serial.print(":");
	Serial.print(bssid[4], HEX);
	Serial.print(":");
	Serial.print(bssid[3], HEX);
	Serial.print(":");
	Serial.print(bssid[2], HEX);
	Serial.print(":");
	Serial.print(bssid[1], HEX);
	Serial.print(":");
	Serial.println(bssid[0], HEX);

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.println(rssi);

	// print the encryption type:
	byte encryption = WiFi.encryptionType();
	Serial.print("Encryption Type:");
	Serial.println(encryption, HEX);
	Serial.println();
}
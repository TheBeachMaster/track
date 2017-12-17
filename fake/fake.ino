#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <SoftwareSerial.h> // Necessary for Arduino Uno

#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);
Adafruit_CC3000_Client cc3000client = Adafruit_CC3000_Client();
int32_t vibrationPinValue;


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883              
#define AIO_USERNAME    ""
#define AIO_KEY         ""



Adafruit_MQTT_Client mqtt(&cc3000client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish readings = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/readings");
Adafruit_MQTT_Publish location_lat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lat");
Adafruit_MQTT_Publish location_long = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/long");
int32_t longitude,latitude;
char WLAN_SSID[] = "Lenovo A7010";
char WLAN_PASS[] =  "Infrared@1944";
#define WLAN_SEC WLAN_SEC_WPA2

void MQTT_connect();

void setup()
{
    randomSeed(analogRead(0));
    Serial.begin(9600);
	while (!Serial)
	{
		;
	}
	cc3000Connect();
}

void loop()
{
vibrationPinValue = random(330, 800);    
longitude = random(32,42);
latitude = random(35);
processReadings(vibrationPinValue, longitude, latitude);
}

void cc3000Connect()
{
	//If we encountered an error Initializing CC3000
	Serial.println(F("Attempting CC300 Init"));
	if (!cc3000.begin()) {
		Serial.println("An error occured Initializing CC3000");
		for (;;);
	}
	//Meanwhile, Delete Old Connection Pofiles and Attempt to Connect to our network
	Serial.println(F("Attempting to delete old profiles"));

	if (!cc3000.deleteProfiles()) {
		Serial.println("We encountered an error Deleting Profiles");
		while (1);
	}
	char* ssid = WLAN_SSID;
	Serial.print("Attempting to connect to ");
	Serial.println(ssid);

	if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SEC)) {
		Serial.println("Could not Connect");
		while (1);
	}

	//Connected
	Serial.println("Connected!");

	Serial.println(F("Request DHCP"));
	while (!cc3000.checkDHCP()) {
		delay(100);
	}
	//Display Connection Details
	displayConnectionDetails();

}

void processReadings(int32_t value, int32_t lon, int32_t lat)
{
    char values[50] ;
    snprintf(values,16,"%lu",value);
    //if value is above 1024/2 something has happened
    if(value >= 500)
    {
        Serial.println("CRITICAL VALUES");
        if (!readings.publish(value) && !location_long.publish(lon) && !location_lat.publish(lat)) {
            Serial.println("ERROR CODE 2"); // failed
          } else {
            Serial.println(values);
          }
    } else
    {
    do{
        Serial.println(values);
        delay(500);
    }while(value < 500);
}
}
bool displayConnectionDetails(void)
{
	uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

	if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
	{
		Serial.println(F("Unable to retrieve the IP Address!\r\n"));
		return false;
	}
	else
	{
		Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
		Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
		Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
		Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
		Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
		Serial.println();
		return true;
	}
}

void MQTT_connect() {
    int8_t ret;
    if (mqtt.connected()) {
      return;
    }
    Serial.println("CONNECTING..");
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) { 
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("RETRYING...");
         mqtt.disconnect();
         delay(5000);
         retries--;
         if (retries == 0) {
           while (1);
         }
    }
    Serial.println("CONNECTED!");
    delay(500);
  }
#define TINY_GSM_MODEM_SIM808
#include <TinyGsmClient.h>
#include <LiquidCrystal.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <SoftwareSerial.h> // Necessary for Arduino Uno

boolean shouldLatch = false; //Kill Motor
byte motorPin = 8; 

#define OK_LED_PIN 13

/*
Setting for The GSM Client
*/
const char apn[]  = "saf";
const char user[] = "safaricom";
const char pass[] = "data";
#define SerialAT Serial1

TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""


/* LCD Circuit Config
The circuit:
* LCD RS pin to digital pin 12
* LCD Enable pin to digital pin 11
* LCD D4 pin to digital pin 5
* LCD D5 pin to digital pin 4
* LCD D6 pin to digital pin 3
* LCD D7 pin to digital pin 2
* LCD R/W pin to ground
* LCD VSS pin to ground
* LCD VCC pin to 5V
* 10K resistor:
* ends to +5V and ground
* wiper to LCD VO pin (pin 3)
*/
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Adafruit_MQTT_Client mqtt(&gsmClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish readings = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/readings");
Adafruit_MQTT_Publish location_lat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lat");
Adafruit_MQTT_Publish location_long = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/long");
int32_t vibrationPinValue;
float longitude, latitude;
void MQTT_connect(); // Resolve ?

void setup(void)
{
    // Set Up LCD
    initiaLizeLCD();
    // debug only...
    Serial.begin(9600);
    while(!Serial)
    {
        ;
    }
    pinMode(OK_LED_PIN, OUTPUT);
    pinMode(motorPin, OUTPUT);
    configureGsm();
}

void loop(void)
{
    MQTT_connect();
    delay(100);
    int data = getVibrationValue();
    data > 500 ? shouldLatch == true : shouldLatch;
    runMotor();
    char * latlong  = getLocation();
    processLocation(latlong);
    processReadings(data, longitude, latitude);
}

int32_t getVibrationValue(void)
{
    vibrationPinValue = analogRead(A0);
    return vibrationPinValue;
}

void initiaLizeLCD(void)
{
     lcd.begin(16, 2);
     lcd.print("LCD Initialized!");
     delay(1500);
     lcd.clear();
}

void processReadings(int32_t value, float lon, float lat)
{
    char values[50] ;
    snprintf(values,16,"%lu",value);
    //if value is above 1024/2 something has happened
    readings.publish(value);
    location_long.publish(lon, 3);
    location_lat.publish(lat, 3);
    if(value >= 500)
    {
        lcd.print("CRITICAL VALUES");
        lcd.print(values);
    }   
        lcd.clear(); // This could cause potential problems .. Remove if need be
        lcd.print(values);
        delay(500);
        lcd.clear();
        delay(1200);
}

void processLocation(char * locPayload)
{
   //  char* gps_data[50] = locPayload;
    // locPayload.toCharArray(gps_data, locPayload.length()+1);
    String lt(locPayload[2]);
    String lg(locPayload[1]);
    double l_t = lt.toFloat()/100;
    double l_g = lg.toFloat()/100;
    longitude = l_g;
    latitude = l_t;
}

void configureGsm(void)
{
  SerialAT.begin(115200);
  delay(3000);
  modem.init(); //Skip to init() avoid long restart()

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  if (!modem.waitForNetwork()) {
    lcd.print("GSM FAILED!");
    delay(1500);
    lcd.clear();
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    while (true);
  }
lcd.print("LOOKS GOOD");
delay(1500);
lcd.clear();
  if (!modem.gprsConnect(apn, user, pass)) {
    lcd.print("GSM FAILURE!");
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    digitalWrite(OK_LED_PIN,1);
    delay(500);
    digitalWrite(OK_LED_PIN,0);
    while (true);
  }
  lcd.clear();
  lcd.print("GSM SUCCESS!");
  delay(1500);
  lcd.clear();
  digitalWrite(OK_LED_PIN,1);
  delay(500);
  digitalWrite(OK_LED_PIN,0);
}

void MQTT_connect() {
    int8_t ret;
    if (mqtt.connected()) {
      return;
    }
    lcd.clear();
    lcd.print("CONNECTING..");
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) { 
         Serial.println(mqtt.connectErrorString(ret));
         lcd.print("RETRYING...");
         mqtt.disconnect();
         delay(5000);
         retries--;
         if (retries == 0) {
           while (1);
         }
    }
    lcd.clear();
    lcd.print("CONNECTED!");
    delay(500);
    lcd.clear();
  }

void runMotor(void)
{
    int speed = 135; // Less than 255
    shouldLatch ? analogWrite(motorPin, 0) : analogWrite(motorPin, speed);
}

#if defined(TINY_GSM_MODEM_SIM808)
char * getLocation(void)
{
    char *  gps_raw  = (char *) malloc (100);
    modem.enableGPS();
    String gps_p = modem.getGPSraw();
    gps_p.toCharArray(gps_raw,100);
    return gps_raw;
    // "0,4043.576433,7400.316980,58.647405,20150601201258.000,64,12,0.548363,100.442406"

}
//   modem.disableGPS();
//   DBG("GPS raw data:", gps_raw);
#endif


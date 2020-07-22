#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <SparkFunTSL2561.h>
#include <Wire.h>
#include <Dht11.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "Settings.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
HTTPClient http;
String ipAddr = "";
String macAddr = "";

SFE_TSL2561 light;
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds

int pin = 2;
Dht11 dht(pin);

int tempF;
float tempC;
int humidity;
double lux;
String luxValue;

unsigned long lastReportTime = 0;
unsigned long lastPollTime = 0;
unsigned long lastErrorTime = 0;
unsigned long startTime = 0;
unsigned long epochTime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(4,5);
  light.begin();
  gain =0;
  unsigned char time = 2;
  light.setTiming(gain, time, ms);
  light.setPowerUp();
  tempF = 0;
  tempC = 0;
  humidity = 0;

  //wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("local_ip: ");
  Serial.println(WiFi.localIP());
  ipAddr = String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
  macAddr = WiFi.macAddress();
  Serial.print("mac: ");
  Serial.println(macAddr);

  //NTP
  timeClient.begin();
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  Serial.println("NTP: " + timeClient.getFormattedTime() + " (" + String(epochTime) + ")");
  startTime = epochTime;
}

void loop() {
  // ntp
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  //poll sensors
  if (epochTime - lastPollTime >= POLL_FREQ_SECONDS) {

      switch (dht.read()) {
        case Dht11::OK:
            Serial.print("Humidity (%): ");
            humidity = dht.getHumidity();
            Serial.println(humidity);
    
            Serial.print("Temperature (C): ");
            tempC = dht.getTemperature();
            Serial.println(tempC);
    
            Serial.print("Temperature (F): ");
            tempF = dht.getTemperature() * 1.8 + 32;
            Serial.println(tempF);
            break;
    
        case Dht11::ERROR_CHECKSUM:
            Serial.println("Checksum error");
            break;
    
        case Dht11::ERROR_TIMEOUT:
            Serial.println("Timeout error");
            break;
    
        default:
            Serial.println("Unknown error");
            break;
    }

    unsigned int data0, data1;
  
    if (light.getData(data0,data1))
    {
      // getData() returned true, communication was successful
      
      Serial.print("data0: ");
      Serial.print(data0);
      Serial.print(" data1: ");
      Serial.print(data1);
  
      boolean good;  // True if neither sensor is saturated
      
      // Perform lux calculation:
  
      good = light.getLux(gain,ms,data0,data1,lux);
      
      // Print out the results:
    
      //Serial.print(" lux: ");
      //Serial.print(lux);
      if (good)
      {
        Serial.print(" lux: ");
        Serial.print(lux);
        Serial.println(" (good)");
      }
      else
      {
        Serial.println(" (BAD)");
      }
    }
    else
    {
      // getData() returned false because of an I2C error, inform the user.
  
      byte error = light.getError();
      printError(error);
    }

    lastPollTime = epochTime;
  }
}

void printError(byte error)
  // If there's an I2C error, this function will
  // print out an explanation.
{
  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error)
  {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}

void report() {
    Serial.print("IDB_R = ");
    http.begin(influxDbUrl);
    http.setAuthorization(influxAuth);

    String strEpoch = String(epochTime) + "000000000";
    String payload = "tempC,location=" + location + ",node=" + node + " value=" + tempC + " " + strEpoch + "\n";
    payload += "tempF,location=" + location + ",node=" + node + " value=" + tempF + " " + strEpoch;

    int httpCode = http.POST(payload);
    http.end();
    Serial.println(String(httpCode) + ":" + epochTime);
}

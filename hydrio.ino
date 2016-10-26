#include <ESP8266WiFi.h>
#include <aREST.h>
#include <SparkFunTSL2561.h>
#include <Wire.h>
#include <Dht11.h>

aREST rest = aREST();

const char* ssid = "ssid";
const char* password = "password";

#define LISTEN_PORT 80
WiFiServer server(LISTEN_PORT);

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(4,5);
  light.begin();
  gain = 1;
  unsigned char time = 2;
  light.setTiming(gain, time, ms);
  light.setPowerUp();
  tempF = 0;
  tempC = 0;
  humidity = 0;
  rest.variable("tempF",&tempF);
  rest.variable("tempC",&tempC);
  rest.variable("humudity",&humidity);
  rest.variable("lux",&luxValue);

  rest.set_id("1");
  rest.set_name("derp");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  
  

}

void loop() {

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
      Serial.println(" (good)");
      luxValue = String(lux);
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

  //Serial.print("Analog soil read: ");
  //Serial.println(analogRead(A0));

    // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
  delay(1000);
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

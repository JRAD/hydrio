#include <SparkFunTSL2561.h>
#include <Wire.h>
#include <Dht11.h>

SFE_TSL2561 light;
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds

int pin = 2;
Dht11 dht(pin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(4,5);
  light.begin();
  gain = 1;
  unsigned char time = 2;
  light.setTiming(gain, time, ms);
  light.setPowerUp();

}

void loop() {
  int temp = 0;
  int humidity = 0;
      switch (dht.read()) {
    case Dht11::OK:
        Serial.print("Humidity (%): ");
        Serial.println(dht.getHumidity());

        Serial.print("Temperature (C): ");
        Serial.println(dht.getTemperature());

        Serial.print("Temperature (F): ");
        Serial.println(dht.getTemperature() * 1.8 + 32);
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

    double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated
    
    // Perform lux calculation:

    good = light.getLux(gain,ms,data0,data1,lux);
    
    // Print out the results:
  
    Serial.print(" lux: ");
    Serial.print(lux);
    if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");
  }
  else
  {
    // getData() returned false because of an I2C error, inform the user.

    byte error = light.getError();
    printError(error);
  }

  Serial.print("Analog soil read: ");
  Serial.println(analogRead(A0));
  
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

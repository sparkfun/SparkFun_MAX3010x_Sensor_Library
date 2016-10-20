/*
  MAX30105 Breakout: Take IR reading to sense presence
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This takes an average reading at power up and if the reading changes more than 100
  then print 'Something is there!'.
  
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

long startTime;
long samplesTaken = 0; //Counter for calculating the Hz or read rate

long unblockedValue; //Average IR at power up

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  particleSensor.setup(0x7F); //Configure sensor. Use 25mA for LED drive
  particleSensor.setPulseAmplitudeRed(0); //Turn off Red LED
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  startTime = millis();

  //Take an average of IR readings at power up
  unblockedValue = 0;
  for (byte x = 0 ; x < 32 ; x++)
  {
    //Wait for new readings to come in
    while (particleSensor.available() == false)
    {
      particleSensor.check(); //Check the sensor, read up to 3 samples
    }

    unblockedValue += particleSensor.getIR(); //Read the IR value
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
  unblockedValue /= 32;

}

void loop()
{
  particleSensor.check(); //Check the sensor, read up to 3 samples

  while (particleSensor.available()) //do we have new data?
  {
    samplesTaken++;

    Serial.print("IR[");
    Serial.print(particleSensor.getIR());
    Serial.print("] Hz[");
    Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
    Serial.print("]");

    long currentDelta = particleSensor.getIR() - unblockedValue;

    Serial.print(" CD[");
    Serial.print(currentDelta);
    Serial.print("]");

    if (currentDelta > (long)100)
    {
      Serial.print(" Something is there!");
    }

    Serial.println();

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
}

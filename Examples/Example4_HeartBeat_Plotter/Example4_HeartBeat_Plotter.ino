/*
  Heart beat plotting!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 20th, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Shows the user's heart beat on Arduino's serial plotter

  Instructions:
  1) Load code onto Redboard
  2) Attach sensor to your finger with a rubber band (see below)
  3) Open Tools->'Serial Plotter'
  4) Make sure the drop down is set to 115200 baud
  5) Checkout the blips!
  6) Feel the pulse on your neck and watch it mimic the blips

  It is best to attach the sensor to your finger using a rubber band or other tightening 
  device. Humans are generally bad at applying constant pressure to a thing. When you 
  press your finger against the sensor it varies enough to cause the blood in your 
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic but requires 5V to power the sensor.
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

void setup()
{
  Serial.begin(115200);

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive

  //Arduino plotter auto-scales annoyingly. To get around this, pre-populate
  //the plotter with 500 of an average reading from the sensor

  //Take an average of IR readings at power up
  const byte avgAmount = 64;
  long baseValue = 0;
  for (byte x = 0 ; x < avgAmount ; x++)
  {
    //Wait for new readings to come in
    while (particleSensor.available() == false)
    {
      particleSensor.check(); //Check the sensor, read up to 3 samples
    }

    baseValue += particleSensor.getIR(); //Read the IR value
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
  baseValue /= avgAmount;

  for (int x = 0 ; x < 500 ; x++)
    Serial.println(baseValue);
}

void loop()
{
  particleSensor.check(); //Check the sensor, read up to 3 samples

  while (particleSensor.available()) //do we have new data?
  {
    Serial.println(particleSensor.getIR());

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
}

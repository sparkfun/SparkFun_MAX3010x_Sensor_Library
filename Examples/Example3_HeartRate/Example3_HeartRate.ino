/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Penpheral Beat Amplitude (PBA) algorithm.


*/

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good. 
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

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
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(0x7F); //Configure sensor. Use 25mA for LED drive
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  particleSensor.check(); //Check the sensor often

  if (particleSensor.available()) //do we have new data?
  {
    if (checkForBeat(particleSensor.getIR()) == true)
    {
      //We sensed a beat!
      
      long delta = millis() - lastBeat;
      lastBeat = millis();

      float beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        Serial.print("BPM: ");
        Serial.print(beatsPerMinute);

        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable

        //Take average of readings
        int beatAvg = 0;
        for(byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
        
        Serial.print(" Avg BPM: ");
        Serial.println(beatAvg);
      }
      else
      {
        Serial.println("Getting reading");
      }
    }

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    
  }
}



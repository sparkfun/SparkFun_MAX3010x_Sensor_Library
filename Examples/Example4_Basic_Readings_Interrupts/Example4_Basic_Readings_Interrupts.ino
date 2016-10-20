/*
  MAX30105 Breakout: Output all the raw Red/IR/Green readings, check INT pin and interrupt register
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Outputs all Red/IR/Green values as fast as possible
  Checks the interrupt pin to see if an interrupt occurred
  Checks the interrupt register to see if a bit was set
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

long startTime;
long samplesTaken = 0; //Counter for calculating the Hz or read rate

byte interruptPin = 3; //Connect INT pin on breakout board to pin 3

void setup()
{
  pinMode(interruptPin, INPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //25mA for LED drive, no sample average, 3 LED mode, sample rate, Pulsewidth of 69, 15-bit
  //particleSensor.setup(0x7F, 1, 3, 200, 69); //At 200Hz serial can keep up, no interrupts
  particleSensor.setup(0x7F, 1, 3, 400, 69); //At 400Hz serial can't keep up and we get interrupts

  particleSensor.enableAFULL(); //Enable the almost full interrupt (default is 32 samples)
  
  particleSensor.setFIFOAlmostFull(3); //Set almost full int to fire at 29 samples

  startTime = millis();
}

void loop()
{
  particleSensor.check(); //Check the sensor, read up to 3 samples

  while (particleSensor.available()) //do we have new data?
  {
    samplesTaken++;

    Serial.print(" R[");
    Serial.print(particleSensor.getRed());
    Serial.print("] IR[");
    Serial.print(particleSensor.getIR());
    Serial.print("] G[");
    Serial.print(particleSensor.getGreen());
    Serial.print("] Hz[");
    Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
    Serial.print("]");

    if (digitalRead(interruptPin) == LOW) //Hardware way of reading interrupts
    {
      Serial.print(" INT!");
    }

    byte flags = particleSensor.getINT1(); //Software way of reading interrupts
    if (flags)
    {
      Serial.print(" I[");
      Serial.print(flags, BIN);
      Serial.print("]");
    }

    Serial.println();

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
}

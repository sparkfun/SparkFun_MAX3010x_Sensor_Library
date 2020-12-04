/*
MAX30101 is an integrated pulse oximetry and heart-rate monitor module.

This code example can be used to run the MAX30101 onboard the EmotiBit.

  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
*/

#include <Wire.h>
#include "MAX30105.h"
#include "wiring_private.h"

TwoWire myWire(&sercom1, 11, 13);
MAX30105 particleSensor;

#define debug Serial //Uncomment this line if you're using an Uno or ESP
//#define debug SerialUSB //Uncomment this line if you're using a SAMD21

void setup()
{
  debug.begin(9600);
  while(!Serial);
  debug.println("EmotiBit MAX30101 Basic Readings Example");
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  myWire.begin();
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
  myWire.setClock(100000);
  Serial.println("Trying to connect to the sensor on I2C line");
  // Initialize sensor
  if (particleSensor.begin(myWire) == false)
  {
    debug.println("FAILURE");
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  else
  {
    debug.println("SUCCESS!");
    //debug.println("open the Arduino Serial Plotter to view data plotted on screen");
  }
  particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive
}

void loop()
{
  //debug.print(" R[");
  debug.print(particleSensor.getRed()); Serial.print("\t");
  //debug.print("] IR[");
  debug.print(particleSensor.getIR());
  //debug.print("] G[");
  //debug.print(particleSensor.getGreen());
  //debug.print("]");

  debug.println();
}

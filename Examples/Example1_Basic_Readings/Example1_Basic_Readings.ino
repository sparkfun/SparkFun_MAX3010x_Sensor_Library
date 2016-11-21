/*
  MAX30105 Breakout: Output all the raw Red/IR/Green readings
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Outputs all Red/IR/Green values.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.

  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

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

  particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive
}

void loop()
{
  Serial.print(" R[");
  Serial.print(particleSensor.getRed());
  Serial.print("] IR[");
  Serial.print(particleSensor.getIR());
  Serial.print("] G[");
  Serial.print(particleSensor.getGreen());
  Serial.print("]");

  Serial.println();
}

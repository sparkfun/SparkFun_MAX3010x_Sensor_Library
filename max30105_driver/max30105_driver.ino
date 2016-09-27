// MAX30105 driver usage example (work in progress)

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

struct colorRecord
{
  long red;
  long IR;
  long green;
};

long startTime;
long samplesTaken = 0; //Counter for calculating the Hz or read rate

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.print("MAX30105 found. (Revision ID = ");
  Serial.print(particleSensor.getRevisionID());
  Serial.println(")");

  particleSensor.setup(0x7F); //Configure sensor. Use 25mA for LED drive

  startTime = millis();

  particleSensor.enableAFULL(); //Enable A_FULL interrupt
}

void loop() {

  samplesTaken += particleSensor.check(); //Check the sensor for new data

  if (particleSensor.available())
  {
    printSamplesFast();
  }

  // Read die temperature
  /*float temp = particleSensor.readTemperature();
    Serial.print("Die Temperature: ");
    Serial.print(temp, 2);
    Serial.print(" deg C");

    temp = particleSensor.readTemperatureF();
    Serial.print(", ");
    Serial.print(temp, 2);
    Serial.println(" deg F");*/

  //delay(100);
}

//Reads in the varies color values and prints them
void printSamplesSlow()
{
  while (particleSensor.available() < 3) particleSensor.check(); //Wait for 3 samples to be available
  
  Serial.print(" R[");
  Serial.print(particleSensor.getRed());
  Serial.print("] IR[");
  Serial.print(particleSensor.getIR());
  Serial.print("] G[");
  Serial.print(particleSensor.getGreen());
  Serial.print("] Hz[");
  Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
  Serial.print("]");

  byte flags = particleSensor.getINT1(); //Read interrupts
  if (flags)
  {
    Serial.print(" I[");
    Serial.print(flags, BIN);
    Serial.print("]");
  }

  Serial.println();
}

//Reads in the varies color values and prints them
void printSamplesFast()
{
  colorRecord temp = particleSensor.getReading();
  
  Serial.print(" R[");
  Serial.print(temp.red);
  Serial.print("] IR[");
  Serial.print(temp.IR);
  Serial.print("] G[");
  Serial.print(temp.green);
  Serial.print("] Hz[");
  Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
  Serial.print("]");

  byte flags = particleSensor.getINT1(); //Read interrupts
  if (flags)
  {
    Serial.print(" I[");
    Serial.print(flags, BIN);
    Serial.print("]");
  }

  Serial.println();
}



//Scans the current sense array
void checkForBeat()
{

}


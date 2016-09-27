// MAX30105 driver usage example (work in progress)

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

const int STORAGE_SIZE = 70; //Each long is 4 bytes so limit this to fit on your micro
struct Record
{
  long red[STORAGE_SIZE];
  long IR[STORAGE_SIZE];
  long green[STORAGE_SIZE];
  byte head;
  byte tail;
};

Record sense; //This is our locally stored readings


long startTime;
long samplesTaken = 0;

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

  zeroSamples(); //Initialize the data set

  startTime = millis();

  particleSensor.enableAFULL(); //Enable A_FULL interrupt
}

void loop() {

  samplesTaken += check(); //Check the sensor for new data

  if (sense.head != sense.tail)
  {
    printSamples();
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


//Zero out all the dataums in the sense struct
void zeroSamples()
{
  for (int x = 0 ; x < STORAGE_SIZE ; x++)
  {
    sense.red[x] = 0;
    sense.IR[x] = 0;
    sense.green[x] = 0;
  }
}

//Prints the struct
void printSamples()
{
  while(sense.tail != sense.head)
  {
    //Serial.print(x);
    Serial.print(" R[");
    Serial.print(sense.red[sense.tail]);
    Serial.print("] IR[");
    Serial.print(sense.IR[sense.tail]);
    Serial.print("] G[");
    Serial.print(sense.green[sense.tail]);
    Serial.print("] Hz[");
    Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
    Serial.print("]");

    byte flags = particleSensor.getINT1(); //Read interrupts
    if(flags)
    {
      Serial.print(" I[");
      Serial.print(flags, BIN);
      Serial.print("]");
    }
    
    Serial.println();

    sense.tail++;
    sense.tail %= STORAGE_SIZE; //Wrap condition
  }
}

//Scans the current sense array 
void checkForBeat()
{
  
}


// MAX30105 driver usage example (work in progress)


//TODO Make library work with different Wire interfaces (SAMD21)

#include <Wire.h>
#include "MAX30105.h"
MAX30105 particleSensor = MAX30105();

struct Record
{
  long red;
  long IR;
  long green;
};

struct Record sense[32]; //These are our stored readings

//activeLEDs is the number of channels turned on, and can be 1 to 3.
//2 is common for Red+IR.
const byte activeLEDs = 2;

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin()) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.print("MAX30105 found. (Revision ID = ");
  Serial.print(particleSensor.getRevisionID());
  Serial.println(")");

  particleSensor.defaultSetup(); //Configure sensor with default settings

  zeroSamples(); //Initialize the data set
}

void loop() {

  int newReadings = check(); //Check the sensor for new data

  if (newReadings > 0)
  {
    Serial.println("New!");
    printSamples();
  }

  // Read die temperature
  float temp = particleSensor.readTemperature();
  Serial.print("Die Temperature: ");
  Serial.print(temp, 2);
  Serial.print(" deg C");

  temp = particleSensor.readTemperatureF();
  Serial.print(", ");
  Serial.print(temp, 2);
  Serial.println(" deg F");

  delay(100);
}

//Polls the sensor for new data
//Call regularly
int check()
{
  //Read register FIDO_DATA in (3-byte * number of active LED) chunks
  //Until FIFO_RD_PTR = FIFO_WR_PTR

  byte readPointer = particleSensor.getReadPointer();
  byte writePointer = particleSensor.getWritePointer();

  int numberOfSamples = 0;

  //Do we have new data?
  if (readPointer != writePointer)
  {
    //Calculate the number of readings we need to get from sensor
    numberOfSamples = writePointer - readPointer;
    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition

    //We now have the number of readings, now calc bytes to read
    //For this example we are just doing Red and IR (3 bytes each)
    int bytesLeftToRead = numberOfSamples * activeLEDs * 3;

    //Get ready to burst read FIFO register
    Wire.beginTransmission(MAX30105_ADDRESS);
    Wire.write(MAX30105_FIFODATA);
    Wire.endTransmission();

    //Wire.requestFrom() is limited to BUFFER_LENGTH which is 32 on the Uno
    //We may need to read as many as 288 bytes so we read in blocks no larger than 32
    //TODO make this work elegantly with 3 channels (max read of 27) and on other platforms (larger buffers)
    while (bytesLeftToRead > 0)
    {
      int toGet = bytesLeftToRead;
      if (toGet > BUFFER_LENGTH)
      {
        //If toGet is 32 this is bad because we read 6 bytes (Red+IR * 3 = 6) at a time
        //32 / 6 = 5 with 2 left over. We don't want to request 32 bytes, we want to request 30.

        toGet = BUFFER_LENGTH - (BUFFER_LENGTH % (activeLEDs * 3)); //Trim toGet to be a multiple of the samples we need to read
      }

      bytesLeftToRead -= toGet;

      //Request toGet number of bytes from sensor
      Wire.requestFrom(MAX30105_ADDRESS, toGet);

      while (toGet > 0)
      {
        byte temp[sizeof(long)];
        long tempLong;

        //Burst read three bytes - RED
        temp[3] = 0;
        temp[2] = Wire.read();
        temp[1] = Wire.read();
        temp[0] = Wire.read();

        //Convert array to long
        memcpy(&tempLong, temp, sizeof(tempLong));

        sense[readPointer].red = tempLong;

        if (activeLEDs > 1)
        {
          //Burst read three more bytes - IR
          temp[3] = 0;
          temp[2] = Wire.read();
          temp[1] = Wire.read();
          temp[0] = Wire.read();

          //Convert array to long
          memcpy(&tempLong, temp, sizeof(tempLong));

          sense[readPointer].IR = tempLong;
        }

        if (activeLEDs > 2)
        {
          //Burst read three more bytes - Green
          temp[3] = 0;
          temp[2] = Wire.read();
          temp[1] = Wire.read();
          temp[0] = Wire.read();

          //Convert array to long
          memcpy(&tempLong, temp, sizeof(tempLong));

          sense[readPointer].green = tempLong;
        }

        toGet -= activeLEDs * 3;

        readPointer++; //Advance our read pointer in the struct
        readPointer %= 32; //Wrap condition
      }

    } //End while (bytesLeftToRead > 0)

  } //End readPtr != writePtr

  return (numberOfSamples); //Let the world know how much new data we found
}

//Zero out all the dataums in the sense struct
void zeroSamples()
{
  for (int x = 0 ; x < 32 ; x++)
  {
    sense[x].red = 0;
    sense[x].IR = 0;
    sense[x].green = 0;
  }
}

//Prints the struct
void printSamples()
{
  for (int x = 0 ; x < 21 ; x++)
  {
    Serial.print(x);
    Serial.print(" R[");
    Serial.print(sense[x].red);
    Serial.print("] IR[");
    Serial.print(sense[x].IR);
    Serial.print("] G[");
    Serial.print(sense[x].green);
    Serial.print("]");
    Serial.println();
  }
}


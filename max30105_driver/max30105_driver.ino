// MAX30105 driver usage example (work in progress)


//TODO Make library work with different Wire interfaces (SAMD21)

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor = MAX30105();

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

//activeLEDs is the number of channels turned on, and can be 1 to 3.
//2 is common for Red+IR.
const byte activeLEDs = 2;

long startTime;
long samplesTaken = 0;

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(&Wire, I2C_SPEED_FAST)) {
//  if (!particleSensor.begin()) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.print("MAX30105 found. (Revision ID = ");
  Serial.print(particleSensor.getRevisionID());
  Serial.println(")");

  particleSensor.defaultSetup(); //Configure sensor with default settings

  zeroSamples(); //Initialize the data set

  startTime = millis();

  particleSensor.enableAFULL(); //Enable A_FULL interrupt
}

void loop() {

  check(); //Check the sensor for new data

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

//Polls the sensor for new data
//Call regularly
//Updates the head and tail in the main struct
//Returns number of new samples obtained
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

        sense.red[sense.head] = tempLong;

        if (activeLEDs > 1)
        {
          //Burst read three more bytes - IR
          temp[3] = 0;
          temp[2] = Wire.read();
          temp[1] = Wire.read();
          temp[0] = Wire.read();

          //Convert array to long
          memcpy(&tempLong, temp, sizeof(tempLong));

          sense.IR[sense.head] = tempLong;
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

          sense.green[sense.head] = tempLong;
        }

        toGet -= activeLEDs * 3;

        readPointer++; //Advance our read pointer in the struct
        readPointer %= 32; //Wrap condition

        sense.head++; //Advance the storage struct in the local processor
        sense.head %= STORAGE_SIZE; //Wrap condition

        samplesTaken++; //Testing
      }

    } //End while (bytesLeftToRead > 0)

  } //End readPtr != writePtr

  return (numberOfSamples); //Let the world know how much new data we found
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


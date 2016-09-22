// MAX30105 driver usage example (work in progress)

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

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin()) {
    Serial.println("MAX30105 was not found.  Please check wiring/power. ");
    while (1);
  }
  Serial.print("MAX30105 found. (Revision ID = ");
  Serial.print(particleSensor.getRevisionID());
  Serial.println(")");

  particleSensor.softReset();

  //FIFO Configuration
  particleSensor.setFIFOAverage(MAX30105_SAMPLEAVG_4); //Guess
  particleSensor.enableFIFORollover(); //All FIFO to wrap/roll over

  //Mode Configuration
  //particleSensor.setLEDMode(MAX30105_MODE_MULTILED); //Let's watch all three LED channels
  particleSensor.setLEDMode(MAX30105_MODE_REDIRONLY); //Red and IR

  //Particle Sensing Configuration
  particleSensor.setADCRange(MAX30105_ADCRANGE_16384); //Guess
  particleSensor.setSampleRate(MAX30105_SAMPLERATE_100); //Guess
  particleSensor.setPulseWidth(MAX30105_PULSEWIDTH_69); //Page 26, Gets us 15 bit resolution

  //LED Pulse Amplitude
  particleSensor.setPulseAmplitudeRed(0x1F); //Guess. Gets us 6.4mA
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0x1F);
  particleSensor.setPulseAmplitudeProximity(0x1F);

  //Multi-LED Mode Configuration, Enable the reading of the three LEDs
  particleSensor.enableSlot(1, SLOT_RED_LED);
  particleSensor.enableSlot(2, SLOT_IR_LED);
  //particleSensor.enableSlot(3, SLOT_GREEN_LED);

  //Read testing:
  zeroSamples();
  readData(2);

  while (1);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(250);
  digitalWrite(13, LOW);
  delay(250);

  // Read die temperature
  float temp = particleSensor.readTemperature();
  Serial.print("Die Temperature: ");
  Serial.print(temp, 2);
  Serial.print(" deg C");

  temp = particleSensor.readTemperatureF();
  Serial.print(", ");
  Serial.print(temp, 2);
  Serial.println(" deg F");
}


//Pulls in some number of bytes from sensor
//activeLEDs is the number of channels turned on, and can be 1 to 3.
//2 is common for Red+IR.
void readData(uint8_t activeLEDs)
{
  particleSensor.clearFIFO(); //Reset the FIFO

  //Read register FIDO_DATA in (3-byte * number of active LED) chunks
  //Until FIFO_RD_PTR = FIFO_WR_PTR

  byte readPointer = particleSensor.readRegister8(MAX30105_ADDRESS, MAX30105_FIFOREADPTR);
  byte writePointer = particleSensor.readRegister8(MAX30105_ADDRESS, MAX30105_FIFOWRITEPTR);

  //For testing
  while (writePointer - readPointer < 30) //Wait for 30 readings to come in
  {
    delay(100);

    readPointer = particleSensor.readRegister8(MAX30105_ADDRESS, MAX30105_FIFOREADPTR);
    writePointer = particleSensor.readRegister8(MAX30105_ADDRESS, MAX30105_FIFOWRITEPTR);
  }

  //Do we have new data?
  if (readPointer != writePointer)
  {
    //Calculate the number of readings we need to get from sensor
    int numberOfSamples = writePointer - readPointer;
    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition

    //We now have the number of readings, now calc bytes to read
    //For this example we are just doing Red and IR (3 bytes each)
    int bytesLeftToRead = numberOfSamples * activeLEDs * 3;

    Serial.print("bytesLeftToRead: ");
    Serial.println(bytesLeftToRead);

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

      //Request toRead number of bytes from sensor
      if (Wire.requestFrom(MAX30105_ADDRESS, toGet) == toGet)
      {
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
      }
      else
      {
        Serial.println("Sensor did not respond with correct # of bytes.");
        Serial.println("Freeze");
        while (1);
      }


    }

    Serial.println("Done!");
    printSamples();

  }
}

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


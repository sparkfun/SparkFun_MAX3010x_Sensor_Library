/*************************************************** 
 This is a library written for the Maxim MAX30105 Optical Smoke Detector

 These sensors use I2C to communicate, as well as a single (optional)
 interrupt line that is not currently supported in this driver.
 
 Written by Peter Janen and XX 
 BSD license, all text above must be included in any redistribution.
 *****************************************************/

#include "MAX30105.h"

MAX30105::MAX30105() {
  // Constructor
  
}

boolean MAX30105::begin(uint8_t i2caddr) {
  Wire.begin();

  _i2caddr = i2caddr; 

  // Step 1: Initial Communciation and Verification
  // Check that a MAX30105 is connected
  if (!readPartID() == MAX_30105_EXPECTEDPARTID) {
    // Error -- Part ID read from MAX30105 does not match expected part ID. 
    // This may mean there is a physical connectivity problem (broken wire, unpowered, etc). 
    return false;
  }

  // Populate revision ID
  readRevisionID();
  
  
  
  
  return true;
}


//
// Die Temperature
//
float MAX30105::readTemperature() {
  // Step 1: Config die temperature register to take 1 temperature sample
  writeRegister8(_i2caddr, MAX30105_DIETEMPCONFIG, 0x01);
  delay(100);

  // Step 2: Read die temperature register (integer)
  int8_t tempInt = readRegister8(_i2caddr, MAX30105_DIETEMPINT);
  uint8_t tempFrac = readRegister8(_i2caddr, MAX30105_DIETEMPFRAC);  

  // Step 3: Calculate temperature (datasheet pg. 23)
  return (float)tempInt + ((float)tempFrac * 0.0625);
}


//
// Device ID and Revision
//
uint8_t MAX30105::readPartID() {
  return readRegister8(_i2caddr, MAX30105_PARTID);
}

void MAX30105::readRevisionID() {  
  revisionID = readRegister8(_i2caddr, MAX30105_REVISIONID);  
}

uint8_t MAX30105::getRevisionID() {
  return revisionID;
}


//
// Low-level I2C Communication 
//
uint8_t MAX30105::readRegister8(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, 1);   // Request 1 byte
  return ( Wire.read() ); 
}

void MAX30105::writeRegister8(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);  
  Wire.write(value);
  Wire.endTransmission();  
}



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

  //Enable the reading of the three LEDs
  //enableSlot(1, SLOT_RED_LED);
  //enableSlot(2, SLOT_GREEN_LED);
  //enableSlot(3, SLOT_IR_LED);

  return true;
}

//
// Configuration
//

void MAX30105::softReset() {
  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_MODECONFIG);

  // Soft reset using the internal reset function (datasheet pg. 19)
  writeRegister8(_i2caddr, MAX30105_MODECONFIG, originalContents | 0x40);

  // Poll for bit to clear, reset is then complete
  // Timeout after 100ms
  unsigned long startTime = millis();
  while (millis() - startTime < 100)
  {
    uint8_t response = readRegister8(_i2caddr, MAX30105_MODECONFIG);
    if ((response & 0x40) == 0) break; //We're done!
    delay(1); //Let's not over burden the I2C bus
  }
}

void MAX30105::shutDown() {
  // Put IC into low power mode (datasheet pg. 19)
  // During shutdown the IC will continue to respond to I2C commands but will
  // not update with or take new readings (such as temperature)

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_MODECONFIG);

  // Step 2: Change contents
  writeRegister8(_i2caddr, MAX30105_MODECONFIG, originalContents | 0x80);
}

void MAX30105::wakeUp() {
  // Pull IC out of low power mode (datasheet pg. 19)

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_MODECONFIG);

  // Step 2: Change contents
  writeRegister8(_i2caddr, MAX30105_MODECONFIG, originalContents & 0x7F);
}


void MAX30105::setLEDMode(uint8_t mode) {
  // Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
  // See datasheet, page 19
  // Note, this code is not currently shutdown aware. (If MAX30105_MODECONFIG is in SHDN, this would bring it out of shutdown)

  switch (mode) {
    case (MAX30105_MODE_REDONLY):
      writeRegister8(_i2caddr, MAX30105_MODECONFIG, MAX30105_MODE_REDONLY);
      break;

    case (MAX30105_MODE_REDIRONLY):
      writeRegister8(_i2caddr, MAX30105_MODECONFIG, MAX30105_MODE_REDIRONLY);
      break;

    case (MAX30105_MODE_MULTILED):
      writeRegister8(_i2caddr, MAX30105_MODECONFIG, MAX30105_MODE_MULTILED);
      break;

    default:
      // this should never happen
      break;
  }
}


void MAX30105::setADCRange(uint8_t adcRange) {
  // adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_PARTICLECONFIG);
  originalContents = originalContents & MAX30105_ADCRANGE_MASK;

  // Step 2: Change contents
  writeRegister8(_i2caddr, MAX30105_PARTICLECONFIG, originalContents | adcRange);
}

void MAX30105::setSampleRate(uint8_t sampleRate) {
  // sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_PARTICLECONFIG);
  originalContents = originalContents & MAX30105_SAMPLERATE_MASK;

  // Step 2: Change contents
  writeRegister8(_i2caddr, MAX30105_PARTICLECONFIG, originalContents | sampleRate);
}

void MAX30105::setPulseWidth(uint8_t pulseWidth) {
  // pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents = readRegister8(_i2caddr, MAX30105_PARTICLECONFIG);
  originalContents = originalContents & MAX30105_PULSEWIDTH_MASK;

  // Step 2: Change contents
  writeRegister8(_i2caddr, MAX30105_PARTICLECONFIG, originalContents | pulseWidth);
}


// NOTE: Amplitude values: 0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical)
// See datasheet, page 21
void MAX30105::setPulseAmplitudeRed(uint8_t amplitude) {
  writeRegister8(_i2caddr, MAX30105_LED1_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeIR(uint8_t amplitude) {
  writeRegister8(_i2caddr, MAX30105_LED2_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeGreen(uint8_t amplitude) {
  writeRegister8(_i2caddr, MAX30105_LED3_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeProximity(uint8_t amplitude) {
  writeRegister8(_i2caddr, MAX30105_LED_PROX_AMP, amplitude);
}

void MAX30105::setProximityThreshold(uint8_t threshMSB) {
  // Set the IR ADC count that will trigger the beginning of particle-sensing mode.
  // The threshMSB signifies only the 8 most significant-bits of the ADC count.
  // See datasheet, page 24.
  writeRegister8(_i2caddr, MAX30105_PROXINTTHRESH, threshMSB);
}

//Given a slot number assign a thing to it
//Devices are SLOT_RED_LED or SLOT_RED_PILOT (proximity)
//Assigning a SLOT_RED_LED will pulse LED
//Assigning a SLOT_RED_PILOT will ??
void MAX30105::enableSlot(uint8_t slotNumber, uint8_t device) {

  // Step 1: Grab current register context, and zero-out the portions of the register we're interested in
  uint8_t originalContents;

  //If we are dealing with Slots 1 or 2, we need register Config1 0x11
  if(slotNumber == 1 || slotNumber == 2) originalContents = readRegister8(_i2caddr, MAX30105_MULTILEDCONFIG1);
  else originalContents = readRegister8(_i2caddr, MAX30105_MULTILEDCONFIG2); //Else Config2

  //If we're dealing with slot 1 or 3, our mask is 0x07
  if(slotNumber == 1 || slotNumber == 3) originalContents = originalContents & MAX30105_SLOT1_MASK;
  else originalContents = originalContents & MAX30105_SLOT2_MASK; //Else mask is 0x70

  //If we're in slots 2 or 4, we need to shift our thing
  if(slotNumber == 2 || slotNumber == 4) device <<= 4; //Line this thing up with its slot 

  originalContents |= device; //Mask in the thing we want to change

  // Step 2: Change contents
  if(slotNumber == 1 || slotNumber == 2) writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG1, originalContents);
  else writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG2, originalContents);
}

//Clears all slot assignments
void MAX30105::disableSlots(void) {
  writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG1, 0);
  writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG2, 0);
}

//
// Data Collection
//

//
// Die Temperature
// Returns temp in C
float MAX30105::readTemperature() {
  // Step 1: Config die temperature register to take 1 temperature sample
  writeRegister8(_i2caddr, MAX30105_DIETEMPCONFIG, 0x01);

  // Poll for bit to clear, reading is then complete
  // Timeout after 100ms
  unsigned long startTime = millis();
  while (millis() - startTime < 100)
  {
    uint8_t response = readRegister8(_i2caddr, MAX30105_DIETEMPCONFIG);
    if ((response & 0x01) == 0) break; //We're done!
    delay(1); //Let's not over burden the I2C bus
  }
  //TODO How do we want to fail? With what type of error?
  //? if(millis() - startTime >= 100) return(-999.0);

  // Step 2: Read die temperature register (integer)
  int8_t tempInt = readRegister8(_i2caddr, MAX30105_DIETEMPINT);
  uint8_t tempFrac = readRegister8(_i2caddr, MAX30105_DIETEMPFRAC);

  // Step 3: Calculate temperature (datasheet pg. 23)
  return (float)tempInt + ((float)tempFrac * 0.0625);
}

// Returns die temp in F
float MAX30105::readTemperatureF() {
  float temp = readTemperature();

  if (temp != -999.0) temp = temp * 1.8 + 32.0;

  return (temp);
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



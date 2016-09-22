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
  bitMask(MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

  // Poll for bit to clear, reset is then complete
  // Timeout after 100ms
  unsigned long startTime = millis();
  while (millis() - startTime < 100)
  {
    uint8_t response = readRegister8(_i2caddr, MAX30105_MODECONFIG);
    if ((response & MAX30105_RESET) == 0) break; //We're done!
    delay(1); //Let's not over burden the I2C bus
  }
}

void MAX30105::shutDown() {
  // Put IC into low power mode (datasheet pg. 19)
  // During shutdown the IC will continue to respond to I2C commands but will
  // not update with or take new readings (such as temperature)
  bitMask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_SHUTDOWN);
}

void MAX30105::wakeUp() {
  // Pull IC out of low power mode (datasheet pg. 19)
  bitMask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_WAKEUP);
}

void MAX30105::setLEDMode(uint8_t mode) {
  // Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
  // See datasheet, page 19
  bitMask(MAX30105_MODECONFIG, MAX30105_MODE_MASK, mode);
}


void MAX30105::setADCRange(uint8_t adcRange) {
  // adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
  bitMask(MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, adcRange);
}

void MAX30105::setSampleRate(uint8_t sampleRate) {
  // sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
  bitMask(MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, sampleRate);
}

void MAX30105::setPulseWidth(uint8_t pulseWidth) {
  // pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
  bitMask(MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, pulseWidth);
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

  uint8_t originalContents;

  switch(slotNumber) {
    case(1):
      bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, device);
      break;
    case(2):
      bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, device << 8);
      break;
    case(3):
      bitMask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, device);
      break;
    case(4):
      bitMask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT4_MASK, device << 8);
      break;
    default:
      //Shouldn't be here!
      break;
  }
}

//Clears all slot assignments
void MAX30105::disableSlots(void) {
  writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG1, 0);
  writeRegister8(_i2caddr, MAX30105_MULTILEDCONFIG2, 0);
}

//
// Data Collection
//

//Set sample average (Table 3, Page 18)
void MAX30105::setFIFOAverage(uint8_t numberOfSamples) {
  bitMask(MAX30105_FIFOCONFIG, MAX30105_SAMPLEAVG_MASK, numberOfSamples);
}

//Resets all points to start in a known state
//Page 15 recommends clearing FIFO before beginning a read
void MAX30105::clearFIFO(void) {
  writeRegister8(_i2caddr, MAX30105_FIFOWRITEPTR, 0);
  writeRegister8(_i2caddr, MAX30105_FIFOOVERFLOW, 0);
  writeRegister8(_i2caddr, MAX30105_FIFOREADPTR, 0);
}

//Enable roll over if FIFO over flows
void MAX30105::enableFIFORollover(void) {
  bitMask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_ENABLE);
}

//Disable roll over if FIFO over flows
void MAX30105::disableFIFORollover(void) {
  bitMask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_DISABLE);
}

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


//Given a register, read it, mask it, and then set the thing
void MAX30105::bitMask(uint8_t reg, uint8_t mask, uint8_t thing)
{
  // Grab current register context 
  uint8_t originalContents = readRegister8(_i2caddr, reg);

  // Zero-out the portions of the register we're interested in
  originalContents = originalContents & mask; 

  // Change contents
  writeRegister8(_i2caddr, reg, originalContents | thing);
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



/*************************************************** 
 This is a library written for the Maxim MAX30105 Optical Smoke Detector

 These sensors use I2C to communicate, as well as a single (optional)
 interrupt line that is not currently supported in this driver.
 
 Written by Peter Janen and XX 
 BSD license, all text above must be included in any redistribution.
 *****************************************************/

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

//
// MAX30105 I2C Address (7-bit)
//
#define MAX30105_ADDRESS  0x57

//
// MAX30105 I2C Register Addresses
//

// Status Registers
#define MAX30105_INTSTAT1         0x00
#define MAX30105_INTSTAT2         0x01
#define MAX30105_INTENABLE1       0x02
#define MAX30105_INTENABLE2       0x03

// FIFO Registers
#define MAX30105_FIFOWRITEPTR     0x04
#define MAX30105_FIFOOVERFLOW     0x05
#define MAX30105_FIFOREADPTR      0x06
#define MAX30105_FIFODATA         0x07

// Configration Registers
#define MAX30105_FIFOCONFIG       0x08
#define MAX30105_MODECONFIG       0x09
#define MAX30105_SPO2CONFIG       0x0A    // Note, this seems like an error in the datasheet (pg. 11)
#define MAX30105_LED1_PULSEAMP    0x0C
#define MAX30105_LED2_PULSEAMP    0x0D
#define MAX30105_LED3_PULSEAMP    0x0E
#define MAX30105_LED_PROXAMP      0x10
#define MAX30105_MULTILEDCONFIG1  0x11
#define MAX30105_MULTILEDCONFIG2  0x12

// Die Temperature Registers
#define MAX30105_DIETEMPINT       0x1F
#define MAX30105_DIETEMPFRAC      0x20
#define MAX30105_DIETEMPCONFIG    0x21

// Proximity Function Registers
#define MAX30105_PROXINTTHRESH    0x30

// Part ID Registers
#define MAX30105_REVISIONID       0xFE
#define MAX30105_PARTID           0xFF    // Should always be 0x15


//
// MAX30105 Commands
//

// TODO


//
// MAX30105 Other Defines
//
#define MAX_30105_EXPECTEDPARTID  0x15


//
// Driver Class Definition
//

class MAX30105 {
 public: 
  MAX30105(void);

  boolean begin(uint8_t i2caddr = MAX30105_ADDRESS);


  // Detecting ID/Revision
  uint8_t getRevisionID();
  uint8_t readPartID();  

  // Low-level I2C communication
  uint8_t readRegister8(uint8_t address, uint8_t reg);
  void writeRegister8(uint8_t address, uint8_t reg, uint8_t value);

  
 private:
  uint8_t _i2caddr;
  uint8_t revisionID;    

  void readRevisionID();
  
};






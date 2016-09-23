/*************************************************** 
 This is a library written for the Maxim MAX30105 Optical Smoke Detector
 It should also work with the MAX30102. However, the MAX30102 does not have a Green LED.

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
#define MAX30105_ADDRESS          0x57
//Note that MAX30102 has the same I2C address and Part ID

#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000

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

// Configuration Registers
#define MAX30105_FIFOCONFIG       0x08
#define MAX30105_MODECONFIG       0x09
#define MAX30105_PARTICLECONFIG   0x0A    // Note, sometimes listed as "SPO2" config in datasheet (pg. 11)
#define MAX30105_LED1_PULSEAMP    0x0C
#define MAX30105_LED2_PULSEAMP    0x0D
#define MAX30105_LED3_PULSEAMP    0x0E
#define MAX30105_LED_PROX_AMP     0x10
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
#define MAX30105_PARTID           0xFF    // Should always be 0x15. Identical to MAX30102.


//
// MAX30105 Commands
//

// Interrupt configuration (pg 13, 14)
#define MAX30105_INT_A_FULL_MASK    (byte)~0b10000000
#define MAX30105_INT_A_FULL_ENABLE  0x80
#define MAX30105_INT_A_FULL_DISABLE 0x00

#define MAX30105_INT_DATA_RDY_MASK  (byte)~0b01000000
#define MAX30105_INT_DATA_RDY_ENABLE  0x40
#define MAX30105_INT_DATA_RDY_DISABLE 0x00

#define MAX30105_INT_ALC_OVF_MASK  (byte)~0b00100000
#define MAX30105_INT_ALC_OVF_ENABLE  0x20
#define MAX30105_INT_ALC_OVF_DISABLE 0x00

#define MAX30105_INT_PROX_INT_MASK  (byte)~0b00010000
#define MAX30105_INT_PROX_INT_ENABLE  0x10
#define MAX30105_INT_PROX_INT_DISABLE 0x00

#define MAX30105_INT_DIE_TEMP_RDY_MASK  (byte)~0b00000010
#define MAX30105_INT_DIE_TEMP_RDY_ENABLE  0x02
#define MAX30105_INT_DIE_TEMP_RDY_DISABLE 0x00

// FIFO configuration commands (pg 18)
#define MAX30105_SAMPLEAVG_MASK     (byte)~0b11100000
#define MAX30105_SAMPLEAVG_1        0x00
#define MAX30105_SAMPLEAVG_2        0x20
#define MAX30105_SAMPLEAVG_4        0x40
#define MAX30105_SAMPLEAVG_8        0x60
#define MAX30105_SAMPLEAVG_16       0x80
#define MAX30105_SAMPLEAVG_32       0xA0

#define MAX30105_ROLLOVER_MASK      0xEF
#define MAX30105_ROLLOVER_ENABLE    0x10
#define MAX30105_ROLLOVER_DISABLE   0x00

#define MAX30105_A_FULL_MASK        0xF0

// Mode configuration commands (page 19)
#define MAX30105_SHUTDOWN_MASK      0x7F
#define MAX30105_SHUTDOWN           0x80
#define MAX30105_WAKEUP             0x00

#define MAX30105_RESET_MASK         0xBF
#define MAX30105_RESET              0x40

#define MAX30105_MODE_MASK          0xF8
#define MAX30105_MODE_REDONLY       0x02
#define MAX30105_MODE_REDIRONLY     0x03
#define MAX30105_MODE_MULTILED      0x07

// Particle sensing configuration commands (pgs 19-20)
#define MAX30105_ADCRANGE_MASK      0x9F
#define MAX30105_ADCRANGE_2048      0x00
#define MAX30105_ADCRANGE_4096      0x20
#define MAX30105_ADCRANGE_8192      0x40
#define MAX30105_ADCRANGE_16384     0x60

#define MAX30105_SAMPLERATE_MASK    0xE3
#define MAX30105_SAMPLERATE_50      0x00
#define MAX30105_SAMPLERATE_100     0x04
#define MAX30105_SAMPLERATE_200     0x08
#define MAX30105_SAMPLERATE_400     0x0C
#define MAX30105_SAMPLERATE_800     0x10
#define MAX30105_SAMPLERATE_1000    0x14
#define MAX30105_SAMPLERATE_1600    0x18
#define MAX30105_SAMPLERATE_3200    0x1C

#define MAX30105_PULSEWIDTH_MASK    0xFC
#define MAX30105_PULSEWIDTH_69      0x00
#define MAX30105_PULSEWIDTH_188     0x01
#define MAX30105_PULSEWIDTH_215     0x02
#define MAX30105_PULSEWIDTH_411     0x03

//Multi-LED Mode configuration (pg 22)
#define MAX30105_SLOT1_MASK         0xF8
#define MAX30105_SLOT2_MASK         0x8F
#define MAX30105_SLOT3_MASK         0xF8
#define MAX30105_SLOT4_MASK         0x8F

#define SLOT_NONE                   0x00
#define SLOT_RED_LED                0x01
#define SLOT_GREEN_LED              0x02
#define SLOT_IR_LED                 0x03
#define SLOT_NONE_PILOT             0x04
#define SLOT_RED_PILOT              0x05
#define SLOT_GREEN_PILOT            0x06
#define SLOT_IR_PILOT               0x07

//
// MAX30105 Other Defines
//
#define MAX_30105_EXPECTEDPARTID    0x15

//
// Driver Class Definition
//

class MAX30105 {
 public: 
  MAX30105(void);

  boolean begin(uint32_t i2cSpeed = I2C_SPEED_STANDARD, uint8_t i2caddr = MAX30105_ADDRESS);

  // Configuration
  void softReset();
  void shutDown(); 
  void wakeUp(); 

  void setLEDMode(uint8_t mode);

  void setADCRange(uint8_t adcRange);
  void setSampleRate(uint8_t sampleRate);
  void setPulseWidth(uint8_t pulseWidth);

  void setPulseAmplitudeRed(uint8_t value);
  void setPulseAmplitudeIR(uint8_t value);
  void setPulseAmplitudeGreen(uint8_t value);
  void setPulseAmplitudeProximity(uint8_t value);

  void setProximityThreshold(uint8_t threshMSB);

  //Multi-led configuration mode (page 22)
  void enableSlot(uint8_t slotNumber, uint8_t device); //Given slot number, assign a device to slot
  void disableSlots(void);
  
  // Data Collection

  //Interrupta (page 13, 14)
  uint8_t getINT1(void); //Returns the main interrupt group
  uint8_t getINT2(void); //Returns the temp ready interrupt
  void enableAFULL(void); //Enable/disable individual interrupts
  void disableAFULL(void);
  void enableDATARDY(void);
  void disableDATARDY(void);
  void enableALCOVF(void);
  void disableALCOVF(void);
  void enablePROXINT(void);
  void disablePROXINT(void);
  void enableDIETEMPRDY(void);
  void disableDIETEMPRDY(void);

  //FIFO Configuration (page 18)
  void setFIFOAverage(uint8_t samples);
  void enableFIFORollover();
  void disableFIFORollover();
  void setFIFOAlmostFull(uint8_t samples);
  
  //FIFO Reading
  uint8_t getWritePointer(void);
  uint8_t getReadPointer(void);
  void clearFIFO(void); //Sets the read/write pointers to zero

  //Proximity Mode Interrupt Threshold
  void setPROXINTTHRESH(uint8_t val);

  // Die Temperature
  float readTemperature();
  float readTemperatureF();

  // Detecting ID/Revision
  uint8_t getRevisionID();
  uint8_t readPartID();  

  // Default setup
  void defaultSetup();

  // Low-level I2C communication
  uint8_t readRegister8(uint8_t address, uint8_t reg);
  void writeRegister8(uint8_t address, uint8_t reg, uint8_t value);
  
 private:
  uint8_t _i2caddr;
  uint8_t revisionID; 

  void readRevisionID();

  void bitMask(uint8_t reg, uint8_t mask, uint8_t thing);
  
};

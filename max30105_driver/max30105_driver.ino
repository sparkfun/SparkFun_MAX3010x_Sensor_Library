

#include <Wire.h>
#define MAX30105_ADDRESS  0x57

uint8_t readRegister8(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, 1);   // Request 1 byte
  return ( Wire.read() ); 
}





// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  Serial.println("Initializing...");

  Wire.begin();
  
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second

  uint8_t id = readRegister8(MAX30105_ADDRESS, 0xFF);
  uint8_t rev = readRegister8(MAX30105_ADDRESS, 0xFE);
  Serial.print("ID: ");
  Serial.print(id);
  Serial.print("  REV: "); 
  Serial.println(rev);
  
  
}

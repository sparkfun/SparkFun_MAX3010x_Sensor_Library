// MAX30105 driver usage example (work in progress)

#include <Wire.h>
#include "MAX30105.h"
MAX30105 particleSensor = MAX30105();

long startTime;
boolean shutDown = false;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
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

  startTime = millis();
  particleSensor.softReset();

  particleSensor.disableSlots(); //Disable all LED assignments in all slots

  //Per datasheet, slots should be assigned in order (i.e., SLOT2 should not be 
  //enabled if SLOT1 is not enabled)

  //Enable the reading of the three LEDs
  particleSensor.enableSlot(1, SLOT_RED_LED);   
  particleSensor.enableSlot(2, SLOT_GREEN_LED);
  particleSensor.enableSlot(3, SLOT_IR_LED);


}

// the loop function runs over and over again forever
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

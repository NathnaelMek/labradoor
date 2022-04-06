
//test ultrasound

#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code



int sensorOutside;
int sensorInside;
 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

// opens the specified door lock
// parameter lock specifies which servo latch is locked
// provides the ability to open/lock door from only one side or bothsides
// lock = -1 ==> open the outside  the outside 
void openDoor(int lock){
 
  Serial.println("open door");

}


// opens the specified door lock
// parameter lock specifies which servo latch is locked
// provides the ability to open/lock door from only one side or bothsides
void closeDoor(int lock){
  // counterclock wise
  Serial.println("close door");

}

void loop() {
  // put your main code here, to run repeatedly:
  sensorInside = digitalRead(2);
  sensorOutside = digitalRead(4);

 
  
}
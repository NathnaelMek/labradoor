
//test ultrasound

#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code




bool isLatchOpen[] = {false, false}; // stores the status of the latches, are they open or closed



// locks/ unlocks the specified door lock
// parameter lock specifies which servo latch is locked
//parameter action specifies whether the action required is 
// provides the ability to open/lock door from only one side or bothsides
// action = 1 means lock
//actions  = 0 means unlock
void doorLatch(char lock, int action){
 
  switch(lock){
    
    case 'o' : // outside latch
      if(action)
        Serial.println("locked outside latch");
      else
        Serial.println("unlocked outside latch");
      break;


    case 'i' : //inside latch
      if(action)
        Serial.println("locked inside latch");
      else
        Serial.println("unlocked inside latch");
      break;

    case 'b' : //both latches
      if(action)
        Serial.println("locked both latches");
      else
        Serial.println("unlocked both latches");
      break;

  }
    Serial.println("Ulocked  outside latch");
  

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //start the latches in a locked position
  doorLatch('b', 1);

}

void loop() {
  // put your main code here, to run repeatedly:
  sensorInside = digitalRead(2);
  sensorOutside = digitalRead(4);

 
  
}

//test ultrasound

#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code#include <Arduino.h> // remove if using arduino IDE. used for platform IO on VS code

#define DOOROPENTIME 3000 //in milliseconds
#define DOORDELAY 2000 // milliseconds

int sensorOutside;// stores sensor data
int sensorInside;
boolean isDoorOpen = false; // door will be closed at first

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(16,OUTPUT); // motor input A1
  pinMode(17,OUTPUT);  // motor input A2
  pinMode(2, INPUT);  // connect the outside sensor 
  pinMode(4, INPUT);  // connect the inside sensor
}

void openDoor(){
 
  Serial.println("open door");

   // clock wise
  digitalWrite(16, HIGH);
  digitalWrite(17, LOW);

  delay(DOOROPENTIME); /// how long the door opening sequence takes

  //stop motor
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
}


void closeDoor(){
  // counterclock wise
  Serial.println("close door");

  // counterclock wise
  digitalWrite(16, LOW);
  digitalWrite(17, HIGH);

  delay(DOOROPENTIME); /// how long the door closing sequence takes

    //stop motor
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorInside = digitalRead(2);
  sensorOutside = digitalRead(4);

  if(sensorOutside || sensorInside){
    // sensor has sensed motion

    if(!isDoorOpen){// if door is closed
      openDoor();   // tell the motor to open the door
      isDoorOpen = true;
    }
    
    
  }else{

    if(isDoorOpen){// if door is open
      delay(DOORDELAY);
      closeDoor(); //  tell the motor to close the door
      isDoorOpen = false;
    }

    
  }
  
  
}
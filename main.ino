#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <cstdlib>

#define DOOROPENTIME 3000 //in milliseconds
#define DOORDELAY 2000 // milliseconds
#define BLE_TAG_NAME "Labradoor tag 1"
#define CUTOFF 80 // in -db for how strong BLE beacon RSSI signal needs to be (ie RSSI reading threshold)
#define INSIDE_LOCK 14
#define OUTSIDE_LOCK 13
#define HALL_SENSOR 11
#define BUTTON 21
#define NUM_MODES 5

#define STATUS_LED_INSIDE 18 
#define STATUS_LED_OUTSIDE 19

boolean isDoorOpen = false; // door will be closed at first
boolean presenseDetected; // see if there is omething ifront of the gate. to wake up from deep sleep

// lock states
boolean insideLock; 
boolean outsideLock;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period  = 15000;

/* modes
    1 unlocked  : both locks are unlocked and door is open.s 
    2 locked : both locks are locked
    3 inside lock : pet cant go inside. only go outside
    4 outside lock
    5 auto lock : always locked. open if pet is near by
    6 auto lock + weather : always locked (streatch goal)
*/
int currentMode;
String modeNames [] = {"unlocked", "locked", "inside lock", "outside lock", "auto lock", "auto + weather"};




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(16,OUTPUT); // servo 1 input A1
  pinMode(17,OUTPUT);  // Servo 2 input A2
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(HALL_SENSOR, INPUT);
  pinMode(STATUS_LED_INSIDE, OUTPUT);
  pinMode(STATUS_LED_OUTSIDE, OUTPUT);
  BLEDevice::init("ESP32_labradoor");
  currentMode = 1;
  unlockInside();
  unlockInside();
  Serial.print ("{LCD} : boot complete. \n\tmode :  ");
  Serial.println (modeNames[currentMode-1]);


}


void unlockInside(){
  insideLock = false;
  digitalWrite(STATUS_LED_INSIDE, LOW);
  
 // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void lockInside(){
  insideLock = true;
  digitalWrite(STATUS_LED_INSIDE, HIGH);
  // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void unlockOutside(){
  outsideLock = false;
  digitalWrite(STATUS_LED_OUTSIDE, LOW);
 // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void lockOutside(){
  outsideLock = true;
  digitalWrite(STATUS_LED_OUTSIDE, HIGH);
  // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

bool isAtEquilibrium(){
    // checks with the hall sensor to see if gate is in equiibrium position
    // wait for 3 seconds before declaring the the door is not swinging
   
    while (digitalRead(HALL_SENSOR) == HIGH)
    {
        //loop for 3 seonds
    }

    return false;
        
    
}

//checks if the dog tag is near the door
bool checkBleTag(){ 
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(1);
  int best = CUTOFF;
  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    int rssi = device.getRSSI();
    String tag_name = device.getName().c_str();
    if (tag_name.equals(BLE_TAG_NAME) ) {
      delay(100);
      Serial.print("tag signal strength : " );
      Serial.println(rssi);

      // check how stron the bluetooth signal is. we want to trigger the gate when the signal is "strong" enough
      // signal stregth is directy proportional to distance. we can play around and find the suitable cutoff
      if (CUTOFF > abs(rssi))
      {
        return true;
      }
      
    }
  }
  return false;
}


// be able to cycle trough the modes with one button
void modeButtonPressed(){
  // "wake lcd"
  startMillis = millis();
  Serial.println ("modeButtonPressed() : wake LCD");
  Serial.print ("{LCD} : ");
  Serial.println (modeNames[currentMode-1]);
  Serial.println ("15 second timer loop started");
  
  while(digitalRead(BUTTON) == LOW){
    ;// wait until the button press is released
  }
  while(true){
    currentMillis = millis();
    delay(10); // for improved performance. neglagible delay
    
    // if button is pressed again before the timer ends, cycle to the next choice and add 3 seconds
    if(digitalRead(BUTTON) == LOW){
      // wait for button release
      while(digitalRead(BUTTON) == LOW){
        //Serial.println ("let go of the button!");
      }
      if(currentMode == NUM_MODES){
        currentMode = 1;
      }else{
        currentMode = (currentMode + 1);
      }
      Serial.print ("{LCD} : ");
      Serial.println (modeNames[currentMode-1]);
    }

    if ((currentMillis - startMillis) >= period)  //test whether the period has elapsed
    {
      
      Serial.print ("{LCD} : ");
      Serial.println (modeNames[currentMode-1]);
      Serial.println("timer ends. close LCD");
      return;
    }

  }
}



void loop() {

  // use mode button and presence sensor to wake ep32 from deepsleep once it is implemented

  if(digitalRead(BUTTON) == LOW)
    modeButtonPressed();
  
  // check what mode is selected and perform action   
  switch(currentMode){

    case 1 : // unlocked mode
      if(insideLock)
          unlockInside();
      
      if(outsideLock)
        unlockOutside();

    break;
    case 2 :  // locked mode
      if(!insideLock)
        lockInside();
      
      if(!outsideLock)
        lockOutside();

    break;
    case 3 :  // inside lock mode
      if(!insideLock)
        lockInside();
      
      if(outsideLock)
        unlockOutside();
    break;
    case 4 :  // outside lock mode
      if(insideLock)
       unlockInside();
      
      if(!outsideLock)
        lockOutside();
    break;
    case 5 :  // auto mode
      // check presence of pet tag
      if(checkBleTag()){
        Serial.println("{auto} : tag detected and authorized");

        // unlock door
        unlockInside();
        unlockOutside();    

        // wait for 5 seconds so that dog can pass through
        delay(5000);
        // the check for door equilibrim
        //while(!isAtEquilibrium()){

        //}

        // lock door
        lockInside();
        lockOutside();
        
      }



    break;
    case 6 :  // auto + weather mode

    break;
    

    



  
  } 
}
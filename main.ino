/*
 when deep sleeping to save power: store currentMode, insidelock and outside lock in the static ram ,
which is part of the real-time-clock (RTC)if esp goes to deep sleep.
 becuase deep sleep erases data from RAM and anything we want to preserve neeeds to be in 
*/



#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <cstdlib>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define DOOROPENTIME 3000 //in milliseconds
#define DOORSTUCKTIME 100000 //in milliseconds
#define DOORDELAY 2000 // milliseconds
#define BLE_TAG_NAME "Labradoor tag 1"
#define CUTOFF 70 // in -db for how strong BLE beacon RSSI signal needs to be (ie RSSI reading threshold)
#define HALL_SENSOR 15
#define BUTTON 25
#define NUM_MODES 5 // how many modes we want the user to select from
#define ECHO_USS 39
#define TRIG_USS 34
#define HALL_VCC 16

//pin used to shutdown time of flight sensor 
//By default it's pulled high. When the pin is pulled low, 
//the sensor goes into shutdown mode.
#define VL53L0X_SHUTDOWN 6 



#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// create four servo objects 
Servo servo_outside;
Servo servo_inside;

// Published values for SG90 servos; adjust if needed
int minUs = 500;
int maxUs = 2500;
int OUTSIDE_SERVOPIN = 32;
int INSIDE_SERVOPIN = 33;

boolean interrupt_check = false; 



boolean presenseDetected; // see if there is omething ifront of the gate. to wake up from deep sleep

// lock states
boolean insideLock; 
boolean outsideLock;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period  = 5000;// 5 seconds
const unsigned long period2  = 10000;// 5 seconds

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
  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  servo_inside.attach(INSIDE_SERVOPIN, minUs, maxUs);
  servo_outside.attach(OUTSIDE_SERVOPIN, minUs, maxUs);
	servo_outside.setPeriodHertz(50);      // Standard 50hz servo
  servo_inside.setPeriodHertz(50);      // Standard 50hz servo
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(HALL_SENSOR, INPUT_PULLUP);
  pinMode(HALL_VCC, OUTPUT);
  BLEDevice::init("ESP32_labradoor");
  currentMode = 1;

  // unlock both lock when booting from power off (no states exist yet so default to unlocked mode)
  unlockInside();
  unlockOutside();

  // display booting message
  oled.clearDisplay(); // clear display
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 20);       // set position to display
  oled.setTextSize(1); 
  oled.print("boot complete.  \n\tmode :  "); // set text
  oled.println(modeNames[currentMode-1]); // set text
  oled.display();              // display on OLED
  delay(2000);
  oled.ssd1306_command(SSD1306_DISPLAYOFF);//display off to save power

}


void unlockInside(){
  insideLock = false;
	servo_inside.write(0);
  delay(500);
  
  
 // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void lockInside(){
  insideLock = true;
  servo_inside.write(90);
  delay(500);
  
  // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void unlockOutside(){
  outsideLock = false;
  servo_outside.write(0);
  delay(500);
 // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void lockOutside(){
  outsideLock = true;
  servo_outside.write(90);
  delay(500);
  // perform locking motion on the servo
 // reach goal :  then cut power of the servo to save battery
}

void OLED (String text, int size = 2, int x = 0, int y = 20){

  oled.clearDisplay(); // clear display
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(x, y);       // set position to display
  oled.setTextSize(size); 
  oled.println(text); // set text
  oled.display();              // display on OLED
  delay(500);         // wait two seconds for initializing
}


// checks with the hall sensor to see if gate is in equiibrium position
// wait for 3 seconds before declaring the the door is not swinging
bool isAtEquilibrium(){

  //start timer
  unsigned long start = millis();
  unsigned long current = millis();

  // turnon power for the Hall sensor switch
  // power to the swtich is connected to a gpio pin so that we can save power 
  digitalWrite(HALL_VCC, HIGH);

  
  while (digitalRead(HALL_SENSOR) == LOW)
  {
    
    current = millis();
    delay(10);
    if((current - start) > 3000){
      Serial.println("at equilibrium!");
      OLED("at equilibrium!");
      return true;
    }
      //loop for 3 seonds
  }
  digitalWrite(HALL_VCC, LOW);
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
      //Serial.print("tag signal strength : " );
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
void sleepDisplay(Adafruit_SSD1306* display) {
  display->ssd1306_command(SSD1306_DISPLAYOFF);
}

void wakeDisplay(Adafruit_SSD1306* display) {
  display->ssd1306_command(SSD1306_DISPLAYON);
}


// be able to cycle trough the modes with one button
void modeButtonPressed(){
 
  oled.ssd1306_command(SSD1306_DISPLAYON);//display off to save power
  Serial.print ("{OLED screen} : "); // DEBUGGING
  Serial.println (modeNames[currentMode-1]);// DEBUGGING
  Serial.println ("5 second timer loop started");// DEBUGGING
  OLED("MODE:\n" + modeNames[currentMode-1], 1);
  
  while(digitalRead(BUTTON) == HIGH){
    ;// wait until the button press is released
  }
  
  startMillis = millis();
  while(true){
    currentMillis = millis();
    delay(10); // for improved performance. neglagible delay
    
    // if button is pressed again before the timer ends, cycle to the next choice and add 5 seconds
    if(digitalRead(BUTTON) == HIGH){
        // wait for button release
      while(digitalRead(BUTTON) == HIGH){
        ;// wait until the button press is released
      }
        // if button is pressed
        startMillis = millis();
        delay(10);
        currentMillis = millis();
        //Serial.println("timer reset");// DEBUGGING
        
        
        if(currentMode == NUM_MODES){
          currentMode = 1;
        }else{
          currentMode = (currentMode + 1);
        }
        OLED (modeNames[currentMode-1]);
        Serial.println(modeNames[currentMode-1]);
    }

    if ((currentMillis - startMillis) >= period)  //test whether the period has elapsed
    {
      oled.clearDisplay(); // clear display
      oled.setTextColor(WHITE);// set text color
      oled.setCursor(0, 20);// set position to display
      oled.setTextSize(1); 
      oled.println ("mode selected : ");
      oled.setTextSize(2); 
      oled.println (modeNames[currentMode-1]);
      oled.display();// display on OLED
      delay(1500);

      //OLED("diplay shutting off", 1);
      //delay(1000);
      oled.ssd1306_command(SSD1306_DISPLAYOFF);//display off to save power
      return;
    }

  }
}

/* 
[not finished yet]

wait for dog to get away from the door
use the ble RSSI signal strength to measure a decrease in signal strength, which
indicates the pet is away from the door. use CUTOFF RSSI value as threshold
*/
void waitForPet(){
  // scan and pause for a 500 milli seconds
  BLEScan *scan;
  oled.ssd1306_command(SSD1306_DISPLAYON);//display off to save power
  oled.clearDisplay(); // clear display
  oled.setTextColor(WHITE);// set text color
  oled.setCursor(0, 20);// set position to display
  oled.setTextSize(1); 
  oled.println ("waiting for tag to get further away from the door");
  oled.display();// display on OLED
  boolean tagFound = false;

  while(true){
    delay(500);
    scan = BLEDevice::getScan();
    scan->setActiveScan(true);
    BLEScanResults results = scan->start(1);
    for (int i = 0; i < results.getCount(); i++) {
      BLEAdvertisedDevice device = results.getDevice(i);
      int rssi = device.getRSSI();
      String tag_name = device.getName().c_str();
      if (tag_name.equals(BLE_TAG_NAME) ) {
        tagFound = true;
        delay(100);
        // check how stron the bluetooth signal is. we want to trigger the gate when the signal is "strong" enough
        // signal stregth is directy proportional to distance. we can play around and find the suitable cutoff
        if (CUTOFF < abs(rssi)){

        }
        else{
          continue;
        }
        
        
      }
    }

  }
  oled.clearDisplay(); // clear display
  oled.ssd1306_command(SSD1306_DISPLAYOFF);//display off to save power



}




void loop() {

  // use mode button and presence sensor to wake ep32 from deepsleep once it is implemented
  servo_inside.attach(INSIDE_SERVOPIN, minUs, maxUs);
  servo_outside.attach(OUTSIDE_SERVOPIN, minUs, maxUs);
  if(digitalRead(BUTTON) == HIGH)
    modeButtonPressed();


  
  // check what mode is selected and perform action   
  switch(currentMode){

    case 1 : // unlocked mode
      
      Serial.println ("unlocked mode");// DEBUGGING
      if(insideLock)
        unlockInside();
      
      if(outsideLock)
        unlockOutside();
      
    break;
    case 2 :  // locked mode
    Serial.println ("locked mode");
      if(!insideLock || !outsideLock){
        if(isAtEquilibrium()){
        
          if(!insideLock){
            lockInside();
          }  
        
          if(!outsideLock){
            lockOutside();
          }  
        }
        
      }    
      // }else{
      //   // check unlikely case where the locks are locked but the door flap is not where it should be (not in betwwen the locks)
      //   if(digitalRead(HALL_SENSOR) == LOW){
      //     // hall sensor is not triggered
      //     //makeNoise
      //     Serial.println("ERROR : flap not in betwwen the locks!");
      //   }
      // }
      // Serial.print("outsideLock : ");
      // Serial.print(outsideLock);
      // Serial.print("lockInside : ");
      // Serial.println(insideLock);
      
        
    break;
    case 3 :  // inside lock mode
    Serial.println (" inside locked mode");
    if(isAtEquilibrium())
      if(!insideLock)
        lockInside();
      
      if(outsideLock)
        unlockOutside();
    
    break;
    case 4 :  // outside lock mode
    Serial.println ("outside locked mode");
    if(isAtEquilibrium())
      if(!outsideLock)
        lockOutside();

      if(insideLock)
       unlockInside();
          
    break;
    
    case 5 :  // auto mode
      Serial.println ("auto mode");
      // check presence of pet tag
      if(checkBleTag()){
        Serial.println("{auto} : tag detected and authorized"); // Debug
        oled.ssd1306_command(SSD1306_DISPLAYON);//display off to save power
        OLED("tag detected");
        delay(2000);
        oled.clearDisplay();
        oled.ssd1306_command(SSD1306_DISPLAYOFF);//display off to save power


        // unlock door
        unlockInside();
        unlockOutside();    

        // wait for 2 seconds so that dog can pass through
        delay(2000);
        // the check for door equilibrim
        startMillis = millis();
        delay(10);
        while(!isAtEquilibrium()){
        // if door is stuck for more than 10 seconds, show error on OLED
        currentMillis = millis(); 




        }
        // lock door
        lockInside();
        lockOutside();


        waitForPet();

      }

    break;
    case 6 :  // auto + weather mode

    break;
    
  } 
}
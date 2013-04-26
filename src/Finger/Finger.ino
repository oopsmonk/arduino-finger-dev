/*
  Finger
  GTM5110 fingerprint sanner example
  Author: OopsMonk
  Email: oopsmonk@gmail.com
  
*/
#include <SoftwareSerial.h>
#include "gtm5110.h" 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

//triger GPIO for open door lock.
#define _LOCK_0_  8  
#define _LOCK_1_  9
//#define _LOCK_2_  10 //TODO_Add_DOOR

//Arduino SoftwareSerial
#define rxPin 3
#define txPin 2
SoftwareSerial gswSerial(rxPin, txPin); //RX, TX

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);    
  pinMode(_LOCK_0_, OUTPUT);
  pinMode(_LOCK_1_, OUTPUT); 
  gswSerial.begin(9600); //for communicte with GTM5110 Fringerprint
  Serial.begin(115200); //for debug
  gtm_get_serial(&gswSerial);
  
}
boolean isInit = false;
boolean isStop = false;
int isOpen = 1;

//door data 
#define _DOOR_MAX_ 2 //TODO_Add_DOOR
#define _UNUSED_DOOR_ -1
//door map stored enrollment id, 
// if not used value = -1, else enrollment id. 
int m_doorMap[_DOOR_MAX_];

void BlinkLED(int count, int duration)
{
	for(int i = 0 ; i <= count; i++){
		digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(duration);               // wait
		digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
		delay(duration);
	}
}

void initDoor()
{
	for(int i = 0; i < _DOOR_MAX_ ; i++){
		m_doorMap[i] = _UNUSED_DOOR_;
	}
        digitalWrite(_LOCK_0_, LOW);
        digitalWrite(_LOCK_1_, LOW);
        //digitalWrite(_LOCK_2_, LOW);//TODO_Add_DOOR
}
int CheckFreeDoor()
{
	for(int i = 0; i < _DOOR_MAX_ ; i++){
		if(m_doorMap[i] == _UNUSED_DOOR_)
			return i; //return unused door id.
	}
	//all door is used.
	return -100;
}
#define _OPEN_TIME_ 3000
void unLock(int id)
{
  Serial.print("Door opening.... ID : ");
  Serial.println(id);
  
  if(id == 0){ 
     digitalWrite(_LOCK_0_, HIGH);
     delay(_OPEN_TIME_);
     digitalWrite(_LOCK_0_, LOW);
  }else if(id == 1){
     digitalWrite(_LOCK_1_, HIGH);
     delay(_OPEN_TIME_);
     digitalWrite(_LOCK_1_, LOW);
  }/* //TODO_Add_DOOR
  else if(id == 2){
     digitalWrite(_LOCK_1_, HIGH);
     delay(_OPEN_TIME_);
     digitalWrite(_LOCK_1_, LOW)
  }*/
  Serial.println("Door Closed!!!");

}

void doEnrollAction(int id)
{
	//start enroll
	int ret = OnEnroll(id);
	if(ret == ENROLL_EXIST){
		//remove from door mapping
		DoorOpenAndRemove(id);
		Serial.println("ENROLL_EXIST goto END");
	}else if(ret == 0){ //enroll OK.
		//add to mapping
		m_doorMap[id] = id;
		//open door
                unLock(id);
		//BlinkLED(5,500);

	}
}

void DoorOpenAndRemove(int doorID)
{

	//remove from fingerprint db
	OnRemoveEnroll(m_doorMap[doorID]);
	//makr door as unused.
	m_doorMap[doorID] = _UNUSED_DOOR_;
	//open door
        unLock(doorID);
	//BlinkLED(5,500);
}

#define _IDENTIFY_TIMEOUT_ 5000
#define _RETRY_TIMEOUT_ 3000

// the loop routine runs over and over again forever:
void loop() {
  
  BlinkLED(1, 500);

  isOpen = gtm_open();
  
  while(isOpen == 0){

	delay(_RETRY_TIMEOUT_);
	if(!isInit){
		//init door mapping
		initDoor();
		//clear data
		OnRemoveAll();
		isInit = true;
	}
	
	Serial.println("#####Start identify#########");
	
	//idenfity
	int doorID = OnIdentify(_IDENTIFY_TIMEOUT_);
	
	if(doorID >= 0 ){ //enrolled
		if(doorID >= _DOOR_MAX_){
			OnRemoveEnroll(doorID);
			Serial.println("It should not be happend!!");
		}else{
			//open door and remove mapping
			DoorOpenAndRemove(doorID);
			Serial.println("open door and remove mapping");
		}
		goto END_ENROLL;
		
	}else if(doorID == ERROR_FP_NOT_IN_DB){ //not enrolled
		//get unuse door
		int doorID = CheckFreeDoor();
		if(doorID >= 0){

			doEnrollAction(doorID);
			goto END_ENROLL;
		}else{ //door is all using.	
			Serial.println("ALL door is using!!!");
			goto END_ENROLL;
		}
	}else if(doorID == ERROR_DB_EMPTY){ //The first user
		doEnrollAction(0);
	}

	
#if 0 //test code for Identify
	int id = OnIdentify(5000);
	Serial.print("Identify ID = ");
	Serial.println(id);
#endif

#if 0 //test code for Enroll and remove 
	//get fingerprint count
	int cnt = gtm_enroll_count();
	Serial.print("enroll count = ");
	Serial.println(cnt);
	int erollID = 2;
   int ret = OnEnroll(erollID);
   if(ret == 0){
	   delay(2000);
	   ret = OnRemoveEnroll(erollID);
	   Serial.print("Remove Enroll = ");
	   Serial.println(ret);
   }
#endif
	END_ENROLL:
		Serial.println("####Restart####");
  }
  
}

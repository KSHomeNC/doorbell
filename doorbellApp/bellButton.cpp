#include "bellButton.h"
#include "GTimer.h"
#include <arduino.h>

uint32_t timerExpiration_us = (1*1000*100 ); // one Sec
const int buttonPin = 22;

void timeHandler(uint32_t data);
bool getButtonStatus();

int ledSts = 0;
bool buttonPressed = false;

void setLED(){
  digitalWrite(LED_B,true);
}

void resetLED(){
  digitalWrite(LED_B, false);
}

void timeHandler(uint32_t data)
{  
  if(getButtonStatus()){
    buttonPressed=true;
    setLED()  ;
  }
}

int initButton()
{
  pinMode(buttonPin, INPUT);
  pinMode(LED_B, OUTPUT);
  GTimer.begin(0, timerExpiration_us, timeHandler, true, 0);
  //digitalSetIrqHandler(buttonPin, button_handler);
  return 0;
}

bool isButtonPressed()
{
  bool sts = buttonPressed;
  buttonPressed = false; // clear after reading 
  resetLED();
  return sts;
}

// time expier @100 mSec 
// check 3 contigues time to confirm button pressed
// stop checking the button pressed status for 10 sec
#define stopTime 10*10 // 10 times of 100msec with 10 times = 10sec


bool getButtonStatus()
{
  bool retVal = false;
  static int count=0;
  static int stopSensingDuration = stopTime;
  static bool stopSensing = false;

  if(stopSensing == false){
    stopSensingDuration = stopTime;
    if(digitalRead(buttonPin)) {
      count++;
    }
    else{
      count=0;
    }

    if(count==3){
      count=0;
      stopSensing = true; 
      retVal = true;
    }
  }
  else{
    if(!stopSensingDuration--){
      stopSensing = false;      
    }
  }
  return retVal;
}


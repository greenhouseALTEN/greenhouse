#include "Miscellaneous.h"
#include "Lighting.h"

Lighting* Lighting::ledLights = 0;          //Initialize pointer to zero so that it can be initialized in first call to getInstance.
Lighting* Lighting::getInstance(){          //Getting the singelton instance of class. Making sure to not create multiple objects of the class by checking if an object previously has been created and in this case use that one instead of creating a new one.
  if (ledLights == 0)
  {
    ledLights = new Lighting();
  }
  return ledLights;
}

/*
==================================================================================================
|| Check current clock time and the current light need to enable/disable start of LED lighting. ||
================================================================================================== */
bool Lighting::checkLightNeed(unsigned short uvValue) {
  //Check if current time is inside specified time interval: 06:31 - 23:31 when LED lighting is allowed to be ON.
  bool enabled = false;
  bool insideTimeInterval = false;
  
  if(currentClockTime >= 631 && currentClockTime < 2332) {
    insideTimeInterval = true;   
  } else {
    insideTimeInterval = false;   
  }
  
  //Check if measured light value is below light threshold value.
  if(insideTimeInterval == true) {
    if(uvValue < UV_THRESHOLD_VALUE) {
      enabled = true;                         //Enable LED lighting to be turned on.
    }
    else {
      enabled = false;                        //Disable LED lighting from being turned on.
    }
  }
  Serial.println("Check light need");
  return enabled;
}

/*
===========================
|| Turn ON LED lighting. ||
=========================== */
bool Lighting::startLed() {
  bool state = false;

  if(ledLightEnabled == true) {
//    relay.turn_on_channel(LED_LIGHTING);                        //Turn on LED lighting.
    state = true;                                               //Update current LED lighting state, 'true' means lighting is turned ON.
    Serial.println("LED lighting ON");
  }
  return state;                                                                                                  
}

/*
============================
|| Turn OFF LED lighting. ||
============================ */
bool Lighting::stopLed() {
  bool state = true;

  if(ledLightEnabled == false) {
//    relay.turn_off_channel(LED_LIGHTING);                       //Turn off LED lighting.
    state = false;                                              //Update current LED lighting state, 'false' means lighting is turned OFF.
    Serial.println("LED lighting OFF");
  }
  return state;
}

/*
==============================================
|| Check if LED lighting is working or not. ||
============================================== */
  //Alarm if light read out value does not get above light threshold when LED lighting is turned on.
bool Lighting::ledLightCheck(unsigned short uvValue) {  
  bool fault = false;
  if(ledLightState == true && uvValue < UV_THRESHOLD_VALUE) {        
    fault = true;                             //If read out light value does not get above light threshold (lower light limit), fault variable is set to 'true' to alert user.
  }
  else {
    fault = false;
  }
  Serial.println("Check LED lighting fault");
  return fault;
}

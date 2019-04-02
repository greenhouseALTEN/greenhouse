#include "Watering.h"
#include "Miscellaneous.h"


/*
==============================
|| Read water level switch. ||
============================== */
bool Watering::readWaterLevel() {
  bool fault = false;
  
  fault = digitalRead(waterLevelSwitch);    //If variable is 'false' water level is OK. If set 'true' water tank level is too low.
  return fault;
}        

/*
===========================================================================================
|| Count number of rotations on flow sensor, runs every time interrupt pin is triggered. ||
=========================================================================================== */
void Watering::flowCount() {
  //Interrupt function to count number of rotations that flow sensor propeller makes when water is being pumped.
  rotations++;
}

/*
===============================================
|| Start water pump, read water flow sensor. ||
=============================================== */
bool Watering::startPump(unsigned short *waterFlowValue) {
  bool state = false;
                  
//  relay.turn_on_channel(WATER_PUMP);            //Start water pump.
  state = true;                                 //Update current water pump state, 'true' means water pump is running.
  Serial.println("Water pump ON");

  //Calculate water flow (Liter/hour) by counting number of rotations that flow sensor makes. Water flow sensor is connected to interrupt pin.
  if(waterPumpState == true) {                  //Only check water flow when water pump is running.
    rotations = 0;                        
    delay(1000);                                //Count number of rotations during one second to calculate water flow in Liter/hour. 
    *waterFlowValue = (rotations * 60) / 7.5;   //Calculate the flow rate in Liter/hour.
  }
  else if(waterPumpState == false) {
    *waterFlowValue = 0;                        //Clearing water flow value when pump is not running to prevent any faulty value from water flow sensor.
  }
  return state;
}

/*
======================
|| Stop water pump. ||
====================== */
bool Watering::stopPump() {
  bool state = true;
  
//  relay.turn_off_channel(WATER_PUMP);   //Stop water pump.
  state = false;                        //Update current water pump state, 'false' means water pump not running.
  Serial.println("Water pump OFF");
  return state;
}

/*
========================================================================
|| Check if water flow is above a certain level when pump is running. ||
======================================================================== */
bool Watering::flowCheck(unsigned short waterFlowValue) {
  bool fault = false;
  
  Serial.println("Check water flow");
  if(waterFlowValue < FLOW_THRESHOLD_VALUE) {   //Check current water flow.
    fault = true;                               //Set fault code.  
    Serial.println("Water flow not OK");
  }
  else {
    fault = false;                              //Clear fault code.
    Serial.println("Water flow OK");
  }
  return fault;
}

/*
======================================
|| Enable/Disable water pump start. ||
====================================== */
bool Watering::checkWaterNeed() {
  //Water pump is enabled if soil moisture is too dry or at the same time as no water related fault codes are set.
  bool enabled = false;
  
  if(moistureDry == true && moistureWet == false) {            
    if(waterLevelFault == false && waterFlowFault == false) {   //Make sure no water related fault codes are set.
      enabled = true;                                           //Enable water pump to run let it start when activated.
    }
    else {
      enabled = false;                                          //Disable water pump to prevent it from starting.  
    }
  }
  Serial.println("Check water need");
  return enabled;
}

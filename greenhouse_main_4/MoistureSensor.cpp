#include "MoistureSensor.h"

/*--------------------------------------------------------//
Moisture sensors.
 */

int MoistureSensor::moistureRead(int moistureSensor, bool *moistureDry, bool *moistureWet) {
  int moistureValue;
  moistureValue = analogRead(moistureSensor);
  
  if(moistureValue <= 300) {
    *moistureDry = true;           //Set warning to display to alert user. Soil too dry.
    *moistureWet = false;
  }
  else if(moistureValue > 300 && moistureValue <= 700) {
    *moistureWet = false;           
    *moistureDry = false;
  }
  else if(moistureValue > 700) {
    *moistureWet = true;           //Set warning to display to alert user. Soil too wet.
    *moistureDry = false;
  }
  return moistureValue;
}

/*
void MoistureSensor::moistureAlarm() {
  //if mean moisture value is to dry.
  //return moistureDry = true;

  //else if mean moisture value is to wet.
  //return moistureWet = true;
}
*/
 
 
 

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

int MoistureSensor::moistureMeanValue(int moistureValue1, int moistureValue2, int moistureValue3, int moistureValue4) {

  //hitta största och minsta värdet och ta bort dessa för att sedan göra mean-value på de två kvarvarande moisture sensorerna.
  //Hur gör med member variables?
  //moistureValue1(moistureValue1), moistureValue2(moistureValue2), moistureValue3(moistureValue3), moistureValue4(moistureValue4)
  //return mean moisture value.
  //return meanMoistureValue;
}

/*
void MoistureSensor::moistureAlarm() {
  //if mean moisture value is to dry.
  //return moistureDry = true;

  //else if mean moisture value is to wet.
  //return moistureWet = true;
}
*/
 
 
 

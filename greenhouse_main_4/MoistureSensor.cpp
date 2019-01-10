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
  int moistureValues[4] = {moistureValue1, moistureValue2, moistureValue3, moistureValue4};
  int moistureMax = 0;                      //Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the highest moisture value.
  int moistureMin = moistureValues[0];      //First value in array of values used as reference value. Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the lowest moisture value.

  //Since four different moisture sensors are used to measure soil moisture in the different post and watering for each individual pots is not possible. The watering action is only based upon a mean value of the moisture readouts. Min and max value are sorted out and not used in case any sensor is not working correctly. 
  for(int i; i<sizeof(moistureValues)/sizeof(int); i++) { //Looping through all measured moisture values to find the highest and lowest moisture values.
    if(moistureMax > moistureValues[i]) {  //Finding the highest measured moisture value.
      moistureMax = moistureValues[i];
    }

    if(moistureMin < moistureValues[i]) {   //Finding the lowest measured moisture value.
      moistureMin = moistureValues[i];
    }
  }
  Serial.print("Max: ");
  Serial.println(moistureMax);
  Serial.print("Min: ");
  Serial.println(moistureMin);
  //hitta största och minsta värdet och ta bort dessa för att sedan göra mean-value på de två kvarvarande moisture sensorerna.
  //Hur gör med member variables?
  //moistureValue1(moistureValue1), moistureValue2(moistureValue2), moistureValue3(moistureValue3), moistureValue4(moistureValue4)
  //moistureMeanValue.
}

/*
void MoistureSensor::moistureAlarm() {
  //if mean moisture value is to dry.
  //return moistureDry = true;

  //else if mean moisture value is to wet.
  //return moistureWet = true;
}
*/
 
 
 

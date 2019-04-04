#include "Moisture.h"
#include "Miscellaneous.h"

/*
===============================================
|| Read moisture value from moisture sensor. ||
=============================================== */
int Moisture::readValue(int moistureSensor) {
  int moistureValue;
  moistureValue = analogRead(moistureSensor);

  return moistureValue;
}
 
/*
==============================================================
|| Calculate moisture mean value from the moisture sensors. ||
============================================================== */
int Moisture::calculateMean(unsigned short moistureValue1, unsigned short moistureValue2, unsigned short moistureValue3, unsigned short moistureValue4) {
  unsigned short moistureValues[4] = {moistureValue1, moistureValue2, moistureValue3, moistureValue4};
  int moistureMax = 0;                                      //Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the highest moisture value.
  int moistureMin = moistureValues[0];                      //First value in array of values used as reference value. Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the lowest moisture value.
  int maxIndex;                                             //Index in array for max moisture value.
  int minIndex;                                             //Index in array for min moisture value.
  int moistureSum = 0;
  int moistureMean;                                         //Stores the moisture mean value before returned to main program.
  
  //Since 4 different moisture sensors are used to measure soil moisture in the four different post and specific watering for each individual pots is not possible. The watering action is only based upon a mean value of the moisture readouts. Min and max value are sorted out and not used in case any sensor is not working correctly. 
  for(int i=0; i<sizeof(moistureValues)/sizeof(int); i++) { //Looping through all measured moisture values to find the highest and lowest moisture values.
    if(moistureValues[i] > moistureMax) {                   //Identify the highest measured moisture value.
      moistureMax = moistureValues[i];
      maxIndex = i;                                         //Identify which moisture sensor that has the max value to be able to delete it from mean moisture value calculation.
    }

    if(moistureValues[i] < moistureMin) {                   //Identify the lowest measured moisture value.
      moistureMin = moistureValues[i];
      minIndex = i;                                         //Identify which moisture sensor that has the min value to be able to delete it from mean moisture value calculation.
    }
  }

  //Remove maximum and minimum moisture values from moisture array.
  moistureValues[minIndex] = 0;                             
  moistureValues[maxIndex] = 0;                             

  for(int i=0; i<sizeof(moistureValues)/sizeof(int); i++) {
    moistureSum += moistureValues[i];                       //Sum the remaining moisture sensor values.
  }
  moistureMean = moistureSum / 2;                           //Calculate mean moisture value with max and min values excluded.

  return moistureMean;
}

/*
===================================
|| Evaluate soil moisture value. ||
=================================== */
void Moisture::evaluateValue(unsigned short moistureMean, bool *moistureDry, bool *moistureWet) {    //CALL THE FUNCTIONS BY PASSING THE ADDRESS OF ITS VALUE &moistureDry and &moistureWet.
  //Evaluate soil humidity based on moisture mean value.
  if(moistureMean <= MOISTURE_THRESHOLD_LOW) {                //Soil humidity is too low.
    *moistureDry = true;                                      //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    *moistureWet = false;
  }
  else if(moistureMean > MOISTURE_THRESHOLD_LOW && moistureMean <= MOISTURE_THRESHOLD_HIGH) { //Soil humidity is good.
    *moistureWet = false;                                     //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    *moistureDry = false;
  }
  else if(moistureMean > MOISTURE_THRESHOLD_HIGH) {           //Soil humidity is to high.
    *moistureWet = true;                                      //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    *moistureDry = false;
  }
}
 

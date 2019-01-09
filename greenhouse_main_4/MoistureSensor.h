#ifndef MoistureSensor_H_
#define MoistureSensor_H_
#include "Arduino.h"
/*------------------------------------------------------// 
Moisture sensors.
*/

class MoistureSensor {
 public:
  int moistureRead(int moistureSensor, bool *moistureDry, bool *moistureWet);
  //Declaring function below with all its variables.
  int MoistureSensor::moistureMeanValue(int moistureValue1, int moistureValue2, int moistureValue3, int moistureValue4);
 private:
  int moistureValue1;
  int moistureValue2;
  int moistureValue3;
  int moistureValue4;
};



#endif  /* MoistureSensor_H_ */

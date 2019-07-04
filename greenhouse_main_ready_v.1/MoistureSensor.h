#ifndef MoistureSensor_H_
#define MoistureSensor_H_
#include "Arduino.h"
#include "Adafruit_seesaw.h"
/*------------------------------------------------------//
  Moisture sensors.
*/

class MoistureSensor {

  Adafruit_seesaw ss;
  
  public:
    void start(byte address)
    {
      ss.begin(address);
    }
  public:
    //Declaring function below with all its variables.
    int moistureRead()
    {
      return ss.touchRead(0);
    }
};



#endif  /* MoistureSensor_H_ */

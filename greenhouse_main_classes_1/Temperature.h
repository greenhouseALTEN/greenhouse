#ifndef Temperature_H_
#define Temperature_H_
#include "Arduino.h"

class Temperature {
public:
  static Temperature *getInstance();                //Return the pointer object of the class.    
  bool thresholdCompare(unsigned short tempValue);
private:
  Temperature(){}                                   //Empty constructor.
  static Temperature *rotaryEncoder;                //Declaring pointer object of the class.
};

#endif  /* Temperature_H_ */

#ifndef Clock_H_
#define Clock_H_
#include "Arduino.h"

class Clock {
public:
  static Clock *getInstance();                    //Return the pointer object of the class. 
  void setTime();
  void resetTime();

private:
  Clock(){}                                       //Empty constructor.
  static Clock *internalClock;                    //Declaring pointer object of the class.
};

#endif  /* Clock_H_ */

#ifndef Temperature_H_
#define Temperature_H_
#include "Arduino.h"

class Temperature {
public:
  static Temperature &getInstance() {
    static Temperature rotaryEncoder;               //Object created on first use. Using getInstance will make sure only one object is created and the same one is used throughout entire program no matter from which part of the program it is called.     
    return rotaryEncoder;                           
  }   
  bool thresholdCompare(unsigned short tempValue);
private:
  Temperature(){}                                   //Empty constructor.
};

#endif  /* Temperature_H_ */

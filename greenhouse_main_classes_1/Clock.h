#ifndef Clock_H_
#define Clock_H_
#include "Arduino.h"

class Clock {
public:
  static Clock &getInstance() {
    static Clock internalClock;                   //Object created on first use. Using getInstance will make sure only one object is created and the same one is used throughout entire program no matter from which part of the program it is called.     
    return internalClock;
  }   
  void setTime();
  void resetTime();

private:
  Clock(){}                                       //Empty constructor.
};

#endif  /* Clock_H_ */

#ifndef Lighting_H_
#define Lighting_H_
#include "Arduino.h"

class Lighting {
public:
  static Lighting &getInstance() {
    static Lighting ledLights;                    //Object created on first use. Using getInstance will make sure only one object is created and the same one is used throughout entire program no matter from which part of the program it is called.     
    return ledLights;                           
  }   
  bool checkLightNeed(unsigned short uvValue);
  bool startLed();
  bool stopLed();
  bool ledLightCheck(unsigned short uvValue);
private:
  Lighting(){}                                    //Empty constructor.
};

#endif  /* Lighting_H_ */

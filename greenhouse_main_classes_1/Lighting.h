#ifndef Lighting_H_
#define Lighting_H_
#include "Arduino.h"

class Lighting {
public:
  static Lighting *getInstance();                 //Return the pointer object of the class.                      
  bool checkLightNeed(unsigned short uvValue);
  bool startLed();
  bool stopLed();
  bool ledLightCheck(unsigned short uvValue);
private:
  Lighting(){}                                    //Empty constructor.
  static Lighting *ledLights;                     //Declaring pointer object of the class.
};

#endif  /* Lighting_H_ */

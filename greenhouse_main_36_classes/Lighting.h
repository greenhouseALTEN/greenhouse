#ifndef Lighting_H_
#define Lighting_H_
#include "Arduino.h"

class Lighting {
public:
  Lighting(){}                                    //Empty constructor.
  bool checkLightNeed(unsigned short uvValue);
  bool startLed();
  bool stopLed();
  bool ledLightCheck(unsigned short uvValue);
private:

};

#endif  /* Lighting_H_ */

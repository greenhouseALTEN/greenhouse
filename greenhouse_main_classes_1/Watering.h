#ifndef Watering_H_
#define Watering_H_
#include "Arduino.h"

class Watering {
public:                          
  bool readWaterLevel();
  static void flowCount();
  bool startPump(unsigned short *waterFlowValue);
  bool flowCheck(unsigned short waterFlowValue);
  bool checkWaterNeed();
  bool stopPump();
  static Watering *getInstance();                 //Function that returns the pointer object of the class.

private: 
  Watering(){}                                    //Empty constructor.
  static Watering *waterPump;                     //Declaring pointer object of the class.
};

#endif  /* Watering_H_ */

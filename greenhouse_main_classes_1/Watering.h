#ifndef Watering_H_
#define Watering_H_
#include "Arduino.h"

class Watering {
public:                          
  static Watering *getInstance();                 //Return the pointer object of the class.
  bool readWaterLevel();
  static void flowCount();
  bool startPump(unsigned short *waterFlowValue);
  bool flowCheck(unsigned short waterFlowValue);
  bool checkWaterNeed();
  bool stopPump();

private: 
  Watering(){}                                    //Empty constructor.
  static Watering *waterPump;                     //Declaring pointer object of the class.
};

#endif  /* Watering_H_ */

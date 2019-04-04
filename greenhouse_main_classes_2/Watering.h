#ifndef Watering_H_
#define Watering_H_
#include "Arduino.h"

class Watering {
public:
  static Watering &getInstance() {
    static Watering waterPump;                    //Object created on first use. Using getInstance will make sure only one object is created and the same one is used throughout entire program no matter from which part of the program it is called.          
    return waterPump;
  }                            
  bool readWaterLevel();
  static void flowCount();
  bool startPump(unsigned short *waterFlowValue);
  bool flowCheck(unsigned short waterFlowValue);
  bool checkWaterNeed();
  bool stopPump();

private:
  Watering(){}                                    //Empty constructor.
};

#endif  /* Watering_H_ */

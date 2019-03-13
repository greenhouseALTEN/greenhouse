#ifndef Watering_H_
#define Watering_H_
#include "Arduino.h"

class Watering {
public:
  Watering(){}                                    //Empty constructor.
  bool readWaterLevel();
  static void flowCount();
  bool startPump(unsigned short *waterFlowValue);
  bool flowCheck(unsigned short waterFlowValue);
  bool checkWaterNeed();
  bool stopPump();

private:

};

#endif  /* Watering_H_ */

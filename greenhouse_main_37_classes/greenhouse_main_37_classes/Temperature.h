#ifndef Temperature_H_
#define Temperature_H_
#include "Arduino.h"

class Temperature {
public:
  Temperature(){}                                   //Empty constructor.
  bool thresholdCompare(unsigned short tempValue);
private:

};

#endif  /* Temperature_H_ */

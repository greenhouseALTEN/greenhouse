#ifndef Moisture_H_
#define Moisture_H_
#include "Arduino.h"

class Moisture {
public:
  Moisture(){}                                    //Empty constructor.
  int readValue(int moistureSensor);
  int calculateMean(unsigned short moistureValue1, unsigned short moistureValue2, unsigned short moistureValue3, unsigned short moistureValue4);
  void evaluateValue(unsigned short moistureMeanValue);
private:

};



#endif  /* Moisture_H_ */

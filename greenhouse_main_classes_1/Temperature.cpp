#include "Miscellaneous.h"
#include "Temperature.h"

Temperature* Temperature::rotaryEncoder = 0;    //Initialize pointer to zero so that it can be initialized in first call to getInstance.
Temperature* Temperature::getInstance(){        //Getting the singelton instance of class. Making sure to not create multiple objects of the class by checking if an object previously has been created and in this case use that one instead of creating a new one.
  if (rotaryEncoder == 0)
  {
    rotaryEncoder = new Temperature();
  }
  return rotaryEncoder;
}

/*
============================================================================================
|| Compare read out temperature with threshold value set by adjustment of rotary encoder. ||
============================================================================================ */
bool Temperature::thresholdCompare(unsigned short tempValue) {
  bool fault = false;
  if(tempValue > (tempThresholdValue / 2)) {      //Compare readout temperature value with temperature threshold value set by rotary encoder. Temp threshold value is divided by 2 to give correct temperature value.
    fault = true;                                 //Readout temperature higher than temperature threshold value, fault variable set to 'true'.
  }
  else {
    fault = false;                                //Readot temperature lower than temperature threshold value, fault variable set to 'false'.
  }
  return fault;
}

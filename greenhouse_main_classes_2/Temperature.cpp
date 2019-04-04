#include "Miscellaneous.h"
#include "Temperature.h"

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

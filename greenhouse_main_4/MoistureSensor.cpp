#include "MoistureSensor.h"

/*
======================================================================
|| Reads moisture value from moisture sensor and returns its value. ||
====================================================================== */
int MoistureSensor::moistureRead(int moistureSensor) {
  int moistureValue;
  moistureValue = analogRead(moistureSensor);

  return moistureValue;
}
 
 
 

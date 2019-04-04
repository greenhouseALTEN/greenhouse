#ifndef Display_H_
#define Display_H_
#include "Arduino.h"
#include "Miscellaneous.h"

class Display {
public:
  static Display *getInstance();                  //Return the pointer object of the class. 
  void printToScreen(unsigned short moistureValue1, unsigned short moistureValue2, unsigned short moistureValue3, unsigned short moistureValue4, unsigned short moistureMeanValue,
                     unsigned short tempValue, unsigned short humidityValue, unsigned short tempThresholdValue, uint16_t lightValue, uint16_t uvValue,
                     bool setButton, bool flashClockCursor);
  static void toggleDisplayMode();
 
private:
  Display() {}                                    //Empty constructor.
  static Display *oledDisplay;                    //Declaring pointer object of the class.
  void viewStartupImage();
  void viewSetClock(bool flashClockCursor);
  void viewReadoutValues(unsigned short moistureMeanValue, unsigned short tempValue, unsigned short humidityValue, unsigned short tempThresholdValue, uint16_t lightValue, uint16_t uvValue);
  void stringToDisplay(unsigned char x, unsigned char y, char* text);
  void numberToDisplay(unsigned char x, unsigned char y, int variable);
  void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks);
  void flashNumberDisplay(unsigned short x, unsigned short y, unsigned short numOfDigits, bool flashClockCursor);
  String soilStatDisplay(bool moistureDry, bool moistureWet); 
};

#endif  /* Display_H_ */

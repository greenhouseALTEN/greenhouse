#ifndef Display_H_
#define Display_H_
#include "Arduino.h"

class Display {
public:
  static Display &getInstance() {
    static Display oledDisplay;                   //Object created on first use. Using getInstance will make sure only one object is created and the same one is used throughout entire program no matter from which part of the program it is called.     
    return oledDisplay;                           
  }   
  void printToScreen(unsigned short moistureValue1, unsigned short moistureValue2, unsigned short moistureValue3, unsigned short moistureValue4, unsigned short moistureMeanValue, unsigned short tempValue, unsigned short humidityValue, uint16_t lightValue, uint16_t uvValue);
  static void toggleDisplayMode();
private:
  Display() {}                                    //Empty constructor.
  void viewStartupImage();
  void viewSetClock();
  void Display::viewReadoutValues(unsigned short moistureMeanValue, unsigned short tempValue, unsigned short humidityValue, uint16_t lightValue, uint16_t uvValue);
  void stringToDisplay(unsigned char x, unsigned char y, char* text);
  void numberToDisplay(unsigned char x, unsigned char y, unsigned short variable);
  void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks);
  void flashNumberDisplay(unsigned short x, unsigned short y, unsigned short numOfDigits);
  void Display::flashCharacterDisplay(unsigned short x, unsigned short y, char symbol);
};

#endif  /* Display_H_ */

/*
*************************
* Included header files *
*************************/
#include "Wire.h"
#include "SeeedOLED.h"
#include "DHT.h"
#include "SI114X.h"
#include "MoistureSensor.h"

/*
****************************************************************
* Pin setup for hardware connected to Arduino UNO base shield. *
****************************************************************/
//Pin setup Arduino UNO board.
#define moistureSensorPort1 A0
#define moistureSensorPort2 A1
#define moistureSensorPort3 A2
#define moistureSensorPort4 A3
#define DHTPIN 4
#define rotaryEncoderOutpA 11
#define rotaryEncoderOutpB 10
#define pumpRelay 8
#define pumpButton 7
#define flowSensor 3
#define waterLevelSwitch 12
#define lightRelay 6

//Arduino UNO base shield layout
/*
################### ARDUINO UNO ###################
#|_____________ANALOG_IN____________POWER________|#
#|             | | | | | | | | | <> | | | | | | ||#
#||A0|      |12C|     |12C|     |12C|     |12C|  |#
#|-----------------------------------------------|#
#||A1|      |D8|      |D7|      |D6|      |D5|   |#
#|-----------------------------------------------|#
#||A2|      |D4|      |D3|      |D2|      |UART| |#
#|-----------------------------------------------|#
#||A3| | | | | | | | | | | | <> | | | | | | | | ||#
#|_____DIGITAL_(PWM_~)___________________________|#
################### ARDUINO UNO ###################
*/

/*
*********************
* Global variables. *
*********************/
//Moisture sensors.
MoistureSensor moistureSensor1;           //Create moistureSensor1 from the MoistureSensor class.
MoistureSensor moistureSensor2;           //Create moistureSensor2 from the MoistureSensor class.
MoistureSensor moistureSensor3;           //Create moistureSensor3 from the MoistureSensor class.
MoistureSensor moistureSensor4;           //Create moistureSensor4 from the MoistureSensor class.
MoistureSensor moistureSensor;            //Create a fictional mean value moisture sensor from the MoistureSensor class.
int moistureValue1;                       //Individual moisture sensor value for moisture sensor 1.
int moistureValue2;                       //Individual moisture sensor value for moisture sensor 2.
int moistureValue3;                       //Individual moisture sensor value for moisture sensor 3.
int moistureValue4;                       //Individual moisture sensor value for moisture sensor 4.                      
int moistureValueMean;                    //Mean value of all 4 moisture sensors.
bool moistureDry = false;                 //Activates warning message on display based on moisture mean value. 'true' if soil for mean value sensor is too dry.
bool moistureWet = false;                 //Activates warning message on display based on moisture mean value. 'true' if soil for mean value sensor is too wet.

//Temperature and humidity sensor.
const uint8_t DHTTYPE = DHT11;            //DHT11 = Arduino UNO model is being used.
DHT humiditySensor(DHTPIN, DHTTYPE);      //Create humidity sensor from DHT class.
float tempValue;
//float humidValue;
bool tempValueFault = false;              //Indicate if read out temperature is higher than temperature treshold that has been set by adjusting temperature rotary encoder. Variable is 'false' when read out temperature is below set temperature threshold.

//Rotary encoder to adjust temperature threshold.
int tempThresholdValue = 60;              //Starting value for temperature threshold adjustment is 30°C.
int aLastState;

//Light sensor.
SI114X lightSensor;                       //Light sensor object created.
uint16_t lightValue;                      //Light read out, probably presented in the unit, lux.
uint16_t uvValue;                       
//uint16_t irValue;                       //IR read out not in use.

//LED lighting.
bool ledLightState = false;               //Indicate current status of LED lighting. Variable is 'true' when LED lighting is turned on.
int uvThresholdValue = 3;                 //UV threshold value for turning LED lighting on/off.
int lightThresholdValue = 0;              //Light threshold value (lux) for turning LED lighting on/off.
bool ledLightFault = false;               //Indicate if LED lighting is not turned on/not working when LED lighting has been turned on.

//Water pump and flow sensor.
volatile int rotations;
int flowValue;
bool pumpState = false;                   //Indicate current status of water pump. Variable is 'true' when water pump is running.
bool waterFlowFault = false;              //Indicate if water is being pumped when water pump is running. Variable is 'false' when water flow is above threshold value. 

//Water level switch.
bool waterLevelValue;                     //If variable is 'false' water level is OK. If 'true' tank water level is too low.

//Delay variables to be used to read relative values the millis()-function. Relative values from millis()-counter are used to print different warning messages to display for a certain amount of time without stopping entire prgram execution, like delay()-function does.
unsigned long timePrev = 0;
unsigned long timeNow;  
unsigned long timeDiff;           //Current time difference from when the warning message function was called. This variable is used to measure for how long time each warning message is shown on display.
int timePeriod = 2100;            //Variable value specifies in milliseconds, for how long time each warning message will be shown on display, before cleared and/or replaced by next warning message.

//Light timer variables.
unsigned long timerStartDark = 0;
unsigned long timerStopDark = 0;
unsigned long timerStartLight = 0;
unsigned long timerStopLight = 0;
unsigned long timeLight = 0;
unsigned long timeDark = 0;
bool day = false;
bool night = false;
int x = 1;


/*
==============================================================
|| Bitmap image 1 to be printed on OLED display at startup. ||
============================================================== */
const unsigned char greenhouse[] PROGMEM= {
  //Startup image 1.
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x60, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x87, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03,
0xff, 0xfc, 0x00, 0x00, 0x00, 0x80, 0xf0, 0x20, 0x00, 0x00, 0x80, 0xc0, 0xc0,
0x60, 0xe0, 0xc0,
0xc0, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xc0, 0x60, 0xe0, 0xc0, 0xc0, 0x80, 0x00,
0x00, 0x80, 0xc0,
0xc0, 0xe0, 0x60, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xc0, 0x60,
0xe0, 0xc0, 0xc0,
0x80, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xe0, 0xe0, 0xc0, 0xc0, 0xf8, 0xf8, 0x00,
0x00, 0x00, 0x00,
0x00, 0xc0, 0xc0, 0xe0, 0x60, 0xc0, 0xc0, 0x80, 0x00, 0xc0, 0xf0, 0xf0, 0xf0,
0xc0, 0x00, 0xc0,
0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xe0,
0xe0, 0xc0, 0xc0,
0xf8, 0xf8, 0x00, 0xd8, 0xd8, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xe0, 0x60, 0xc0,
0xc0, 0x80, 0x00,
0x00, 0x03, 0x0f, 0x1e, 0x3c, 0x70, 0xe3, 0xcf, 0x9f, 0x30, 0x00, 0x00, 0x00,
0x00, 0x70, 0xbf,
0xcf, 0xe3, 0x70, 0x78, 0x3e, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x33, 0x77, 0x66,
0x66, 0x66, 0x6c,
0x7d, 0x18, 0x00, 0x1f, 0x3f, 0x76, 0x66, 0x66, 0x66, 0x76, 0x37, 0x07, 0x00,
0x0f, 0x3f, 0x7f,
0x66, 0x66, 0x66, 0x66, 0x77, 0x27, 0x07, 0x00, 0x1f, 0x3f, 0x76, 0x66, 0x66,
0x66, 0x76, 0x37,
0x07, 0x00, 0x0f, 0x3f, 0x71, 0x60, 0x60, 0x60, 0x60, 0x31, 0x7f, 0x7f, 0x00,
0x00, 0x00, 0x00,
0x11, 0x37, 0x67, 0x66, 0x66, 0x6c, 0x7d, 0x38, 0x00, 0x00, 0x3f, 0x7f, 0x3f,
0x00, 0x00, 0x1f,
0x3f, 0x70, 0x60, 0x60, 0x70, 0x7f, 0x7f, 0x00, 0x0f, 0x3f, 0x71, 0x60, 0x60,
0x60, 0x60, 0x31,
0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x00, 0x06, 0x1f, 0x3b, 0x60, 0x60, 0x60, 0x60,
0x71, 0x3f, 0x1f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x48, 0x48, 0x48, 0xb0,
0x00, 0xc0, 0x20,
0x20, 0x20, 0xc0, 0x00, 0xc0, 0x20, 0x20, 0x20, 0xc0, 0x00, 0x40, 0xa0, 0xa0,
0xa0, 0x20, 0x00,
0x00, 0x20, 0xf0, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
0x08, 0xf8, 0x08,
0x08, 0x00, 0xc0, 0x20, 0x20, 0x20, 0xf8, 0x00, 0xc0, 0xa0, 0xa0, 0xa0, 0xc0,
0x00, 0x20, 0xa0,
0xa0, 0xa0, 0xc0, 0x00, 0x40, 0xa0, 0xa0, 0xa0, 0x20, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x48, 0x48, 0x48, 0x08, 0x00, 0x20,
0x40, 0x80, 0x40,
0x20, 0x00, 0x00, 0x20, 0xf0, 0x20, 0x20, 0x00, 0xc0, 0xa0, 0xa0, 0xa0, 0xc0,
0x00, 0xe0, 0x00,
0x20, 0x20, 0xc0, 0x00, 0xc0, 0x20, 0x20, 0x20, 0xf8, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x01,
0x00, 0x01, 0x02,
0x02, 0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x02, 0x01, 0x00, 0x02, 0x02, 0x02,
0x02, 0x01, 0x00,
0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
0x02, 0x03, 0x02,
0x02, 0x00, 0x01, 0x02, 0x02, 0x02, 0x03, 0x00, 0x01, 0x02, 0x02, 0x02, 0x00,
0x00, 0x01, 0x02,
0x02, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00, 0x00, 0x08, 0x06,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x82, 0x02, 0x00, 0x02,
0x01, 0x01, 0x01,
0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x01, 0x02, 0x02, 0x02, 0x00,
0x00, 0x03, 0x00,
0x00, 0x00, 0x03, 0x00, 0x01, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x82, 0x8c, 0x60, 0x1c, 0x02, 0x00, 0x1c, 0x22, 0x22, 0x22,
0x1c, 0x00, 0x1e,
0x20, 0x20, 0x00, 0x3e, 0x00, 0x00, 0x3e, 0x04, 0x02, 0x02, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x3e, 0x04, 0x02, 0x02, 0x00, 0x1c, 0x2a, 0x2a, 0x2a, 0x0c, 0x00,
0x12, 0x2a, 0x2a,
0x2a, 0x1c, 0x20, 0x1c, 0x22, 0x22, 0x22, 0x14, 0x00, 0x3f, 0x00, 0x02, 0x02,
0x3c, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00 
};

/*
==============================================================
|| Bitmap image 2 to be printed on OLED display at startup. ||
============================================================== */
const unsigned char features[] PROGMEM= {
  //Startup image 2.
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/*
======================================================
|| Initialize OLED display and show startup images. ||
====================================================== */
void startupDisplay() {
  Wire.begin();
  SeeedOled.init();
  SeeedOled.clearDisplay();       //Clear display and set start position to top left corner.
  SeeedOled.setHorizontalMode();
  SeeedOled.setNormalDisplay();   //Set display to normal mode (non-inverse mode).
  SeeedOled.setPageMode();        //Set addressing mode to Page Mode.

/*
  //Show startup images when starting up the system.
  //Startup image 1.
  SeeedOled.drawBitmap(greenhouse, (128*64)/8);   //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
  delay(4000);                                    //Image shown for 4 seconds.
  SeeedOled.clearDisplay();                       //Clear the display.

  //Startup image 2.
  SeeedOled.drawBitmap(features, (128*64)/8);     //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
  delay(3000);                                    //Image shown for 3 seconds.
  SeeedOled.clearDisplay();                       //Clear the display.
*/
}

/*
============================================
|| Print read out values to OLED display. ||
============================================ */
void displayValues() {
  //Clear redundant value figures from previous read out for all sensor values.
  SeeedOled.setTextXY(0, 42);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(1, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(2, 42);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(3, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(4, 42);
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(5, 42);
  SeeedOled.putString("      ");

  //Printing read out values from the greenhouse to display.
  /*
  ************************
  |Moisture sensor value.|
  ************************/
  SeeedOled.setTextXY(0, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Moisture: ");        //Print string to display.
  SeeedOled.setTextXY(0, 42);
  SeeedOled.putNumber(moistureValueMean);   //Print mean moisture value to display.

  /*******************
  |Temp sensor value.|
  ********************/
  SeeedOled.setTextXY(1, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Temp: ");            //Print string to display.
  SeeedOled.setTextXY(1, 42);
  SeeedOled.putNumber(tempValue);           //Print temperature value to display.

  /***********************
  |UV-light sensor value.|
  ************************/
  SeeedOled.setTextXY(2, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("UV-light: ");        //Print string to display.
  SeeedOled.setTextXY(2, 42);
  SeeedOled.putNumber(uvValue);             //Print UV-light value to display.

  /********************
  |Light sensor value.|
  *********************/
  SeeedOled.setTextXY(3, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Light: ");           //Print string to display.
  SeeedOled.setTextXY(3, 42);
  SeeedOled.putNumber(lightValue);          //Print light value in the unit, lux, to display.
  SeeedOled.putString("lm");                 //Print unit of the value.

  /**********************
  |Temp threshold value.|
  ***********************/
  SeeedOled.setTextXY(4, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Temp lim: ");        //Print string to display.
  SeeedOled.setTextXY(4, 42);
  SeeedOled.putNumber(tempThresholdValue / 2);    //Print temperature threshold value to display. Value 24 corresponds to 12°C, temp value is doubled to reduce rotary sensitivity and increase knob rotation precision.
  
  /*************************
  |Water flow sensor value.|
  **************************/
  SeeedOled.setTextXY(5, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Flow Sens: ");       //Print string to display.
  SeeedOled.setTextXY(5, 42);
  SeeedOled.putNumber(flowValue);           //Print water flow value to display.
  SeeedOled.putString("L/h");               //Print unit of the value.

  //Printing separator line to separate read out values from error/warning messages.
  SeeedOled.setTextXY(6, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("----------------");  //Print string to display.
}

/*
==========================================
|| Read light values from light sensor. ||
========================================== */
void lightRead() {
  lightValue = lightSensor.ReadVisible();
  uvValue = lightSensor.ReadUV();
  //irValue = lightSensor.ReadIR();
}

/*
===============================
|| Turn LED lighting on/off. ||
=============================== */
void ledLightStart() {
  if(uvValue < uvThresholdValue) {                                  //Turn on LED lighting if measured UV value is below uvThresholdValue.
    digitalWrite(lightRelay, HIGH);
    ledLightState = true;                                           //Update current status for LED lighting.
  }
  else {
    digitalWrite(lightRelay, LOW);                                  //Turn off LED light if measured UV value is above a certain value.
    ledLightState = false;                                          //Update current status for LED lighting.
  }

  //Alarm if read out light value does not get above light threshold when LED lighting is turned on.
  if(ledLightState == true && uvValue != uvThresholdValue) {        
    ledLightFault = true;                                           //If read out light value does not get above light threshold (lower light limit) when LED lighting is turned on. Fault variable is set to 'true' to alert user.
  }
  else {
    ledLightFault = false;
  }
}

/*
==============================
|| Read water level switch. ||
============================== */
void waterLevelRead() {
  waterLevelValue = digitalRead(waterLevelSwitch);                  //If variable is 'false' water level is OK. If 'true' tank water level is too low.
}

/*
======================================================
|| Start water pump and read out water flow sensor. ||
====================================================== */
void pumpStart() {
  //Calculate water flow (Liter/hour) by counting number of rotations that flow sensor makes. Water flow sensor is connected to interrupt pin.
  rotations = 0;                        
  delay(1000);                          //Count number of rotations during one second to calculate water flow in Liter/hour. 
  flowValue = (rotations * 60) / 7.5;   //Calculate the flow rate in Liter/hour.

  //Starting/stopping pump with a button.
  bool pushButton = digitalRead(pumpButton);
  
  if(pushButton == true) {
    digitalWrite(pumpRelay, HIGH);      //Start water pump if button is being pressed.
    pumpState = true;
  }
  else {
    digitalWrite(pumpRelay, LOW);       //Stop water pump if button not is being pressed.
    pumpState = false;
  }

  //Alarm if no water is being pumped even though water pump is running even though tank water level is ok.
  if(pumpState == true && flowValue < 8 && waterLevelValue == false) {
    waterFlowFault = true;              //If there is no water flow or the flow is too low while water pump is running. Fault variable is set to 'true' to alert user.
  }
  else {
    waterFlowFault = false;
  }
}

/*
===========================================================================================
|| Count number of rotations on flow sensor, runs every time interrupt pin is triggered. ||
=========================================================================================== */
void flowCount() {
  //Interrupt function to count number of rotations that flow sensor makes when water is being pumped.
  rotations++;
}

/*
====================================================================================================
|| Timer interrupt to read temperature threshold value, that is being adjusted by rotary encoder. ||
==================================================================================================== */
ISR(TIMER1_COMPA_vect) {  //Timer interrupt 100Hz to read temperature threshold value set by rotary encoder.
  //Reading preset temperature threshold set by rotation encoder knob.
  int minTemp = 28;   //Temperature value can be set within the boundaries of 14 - 40°C (minTemp - maxTemp). Temp value is doubled to reduce rotary sensitivity and increase knob rotation precision.
  int maxTemp = 80;
  int aState;

  aState = digitalRead(rotaryEncoderOutpA);                                      //Reads the current state of the rotary knob, outputA.
  
  if(aState != aLastState) {                                        //A turn on rotary knob is detected by comparing previous and current state of outputA.
    if(digitalRead(rotaryEncoderOutpB) != aState && tempThresholdValue <= maxTemp) { //If outputB state is different to outputA state, that meant the encoder knob is rotation clockwise.
      tempThresholdValue++;                                         //Clockwise rotation means increasing position value. Position value is only increased if less than max value.
    }
    else if(tempThresholdValue > minTemp) {
      tempThresholdValue--;                                         //Counter clockwise rotation means decreasing position value.
    }
  }
  aLastState = aState;                                              //Updates the previous state of outputA with current state.
}

/*
============================================================================================================
|| Compare read out temperature with temperature threshold that has been set by adjusting rotary encoder. ||
============================================================================================================ */
void tempThresholdCompare() {
  if(tempValue > tempThresholdValue/2) {                            //Compare read out temperature value with temperature threshold value set by rotary encoder. Temp threshold value is divided by 2 to give correct temperature value.
     tempValueFault = true;                                         //If measured temperature is higher than temperature threshold that has been set, variable is set to 'true' to alert user.
  }
  else {
    tempValueFault = false;
  }
}
/*
=============================================================================
|| Print alarm message to display for any fault that is currently active . ||
============================================================================= */
void alarmMessageDisplay() {       
  timeNow = millis();                                   //Read millis() value to be used as delay to present multiple warning messages at the same space of display after another.
  timeDiff = timeNow - timePrev;
  Serial.print("timeDiff: ");
  Serial.println(timeDiff);

  /******************
  |Water flow fault.|
  *******************/
  if(timeDiff <= timePeriod) {
    SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
    SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
    if(waterFlowFault == true) {                        //If fault variable is set to 'true', fault message is printed to display.
      SeeedOled.setTextXY(7, 0);                        //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
      SeeedOled.putString("No water flow!");            //Print fault message to display.
    }

    else {  //If this alarm not active, clear the warning message row.
      SeeedOled.setTextXY(7, 0);                        //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                        ");  //Clear row to enable other warnings to be printed to display.
    }
  }

  /**********************
  |Low water tank level.|
  ***********************/
  if(timePeriod < timeDiff && timeDiff <= timePeriod * 2) {
    if(waterLevelValue == true) {                         //If fault variable is set to 'true', fault message is printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("Refill tank!");                //Print fault message to display.
    }
    else {  //If this alarm not active, clear the warning message row.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.      
    }
  }

  /*******************
  |Temperature fault.|
  ********************/
  if(timePeriod * 2 < timeDiff && timeDiff <= timePeriod * 3) { 
    if(tempValueFault == true) {                          //If fault variable is set to 'true', fault message is printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("Too warm!");                   //Print fault message to display.
    }
    else {  //If this alarm not active, clear the warning message row.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.      
    }
  }

  /********************
  |LED lighting fault.|
  *********************/
  if(timePeriod * 3 < timeDiff && timeDiff <= timePeriod * 4) { 
    if(ledLightFault == true) {                           //If fault variable is set to 'true', fault message is printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("Check LED light!");            //If measured water flow is below a certain value without the water level sensor indicating the water tank is empty, there is a problem with the water tank hose. "Check water hose!" is printed to display.
    }
    else {  //If this alarm not active, clear the warning message row.
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.      
    }
  }
  
  if(timePeriod * 4 < timeDiff) {
    SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
    SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
    timePrev = millis();                                //Loop warning messages from start.
  } 
}

/*
>>>>>>>>>NOT COMPLETED AND NOT FULLY DEFINED!!!!!!!!!!!!!!!!!!
======================================================================================
|| Timer to measure the time for how plants have been exposed to lightess/darkness. ||
====================================================================================== */
void lightTimer(uint16_t uvValue, uint16_t lightValue) {
  //Measure how much light plants have been exposed to during every 24 hours period. It then turns on/off the LED strip lighting to let plants rest 6 hours in every 24 hours period.
  if(x == 1) {
    delay(3000);
    x = 0;
  }
  
  if(uvValue < 3 && day == false) {
    timerStartDark = millis();
    day = true;
  }
  else if(uvValue > 3 && day == true) {
    timerStopDark = millis();
    timeDark += timerStopDark - timerStartDark;
    //Serial.print("timeDark: ");
    //Serial.println(timeDark);
    day = false;
  }

  if(uvValue > 3 && night == false) {
    timerStartLight = millis();
    night = true;
  }
  else if(uvValue < 3 && night == true) {
    timerStopLight = millis();
    timeLight += timerStopLight - timerStartLight;
    //Serial.print("timeLight: ");
    //Serial.println(timeLight);
    night = false;
  }

  if(timeDark + timeLight == 10000) {
    Serial.print("totalLight: ");
    Serial.println(timeLight);
    Serial.print("totalDark: ");
    Serial.println(timeDark);
  }
}

/*
=====================================================================================================
|| Calculate moisture mean value based on measured moisture values from all four moisture sensors. ||
===================================================================================================== */
int moistureMeanValue(int moistureValue1, int moistureValue2, int moistureValue3, int moistureValue4) {
  int moistureValues[4] = {moistureValue1, moistureValue2, moistureValue3, moistureValue4};
  int moistureMax = 0;                                      //Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the highest moisture value.
  int moistureMin = moistureValues[0];                      //First value in array of values used as reference value. Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the lowest moisture value.
  int maxIndex;                                             //Index in array for max moisture value.
  int minIndex;                                             //Index in array for min moisture value.
  int moistureSum = 0;
  int moistureMeanValue;                                    //Stores the moisture mean value before returned to main program.
  
  //Since 4 different moisture sensors are used to measure soil moisture in the four different post and specific watering for each individual pots is not possible. The watering action is only based upon a mean value of the moisture readouts. Min and max value are sorted out and not used in case any sensor is not working correctly. 
  for(int i=0; i<sizeof(moistureValues)/sizeof(int); i++) { //Looping through all measured moisture values to find the highest and lowest moisture values.
    if(moistureValues[i] > moistureMax) {                   //Identify the highest measured moisture value.
      moistureMax = moistureValues[i];
      maxIndex = i;                                         //Identify which moisture sensor that has the max value to be able to delete it from mean moisture value calculation.
    }

    if(moistureValues[i] < moistureMin) {                   //Identify the lowest measured moisture value.
      moistureMin = moistureValues[i];
      minIndex = i;                                         //Identify which moisture sensor that has the min value to be able to delete it from mean moisture value calculation.
    }
  }

  //Remove maximum and minimum moisture value from moisture array.
  moistureValues[minIndex] = 0;                             
  moistureValues[maxIndex] = 0;                             

  for(int i=0; i<sizeof(moistureValues)/sizeof(int); i++) {
    moistureSum += moistureValues[i];                       //Sum the remaining moisture sensor values.
  }
  moistureMeanValue = moistureSum / 2;                      //Calculate mean moisture value with max and min values excluded.
    
  if(moistureMeanValue <= 300) {
    moistureDry = true;                                     //Set warning to display to alert user. Soil too dry.
    moistureWet = false;
  }
  else if(moistureMeanValue > 300 && moistureMeanValue <= 700) {
    moistureWet = false;           
    moistureDry = false;
  }
  else if(moistureMeanValue > 700) {
    moistureWet = true;                                     //Set warning to display to alert user. Soil too wet.
    moistureDry = false;
  }
  return moistureMeanValue;
}


/*
*******************************
* Arduino program setup code. *
*******************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  startupDisplay();                       //Initialize the OLED Display and show startup images.
  
  pinMode(moistureSensorPort1, INPUT);
  pinMode(moistureSensorPort2, INPUT);
  pinMode(moistureSensorPort3, INPUT);
  pinMode(moistureSensorPort4, INPUT);
  
  pinMode(flowSensor, INPUT);
  attachInterrupt(1, flowCount, RISING);  //Initializing interrupt to enable water flow sensor to calculate water flow pumped by water pump.
 
  pinMode(pumpRelay, OUTPUT);
  pinMode(pumpButton, INPUT);
  
  pinMode(waterLevelSwitch, INPUT);
  
  pinMode(lightRelay, OUTPUT);
  
  pinMode(rotaryEncoderOutpA, INPUT);
  pinMode(rotaryEncoderOutpB, INPUT);
  aLastState = digitalRead(rotaryEncoderOutpA);      //Read initial position value.
  
  humiditySensor.begin();                 //Initializing humidity sensor.
  
  lightSensor.Begin();                    //Initializing light sensor.

  //Enable time interrupt to read temperature threshold value set by rotary encoder.
  cli();  //Stop interrupts.
  //Set timer1 interrupt at 100Hz
  TCCR1A = 0;     //Set entire TCCR1A register to 0.
  TCCR1B = 0;     //Set entire TCCR1B register to 0.
  TCNT1 = 0;      //Initialize counter value to 0.
  OCR1A = 156;    //match reg. = 16MHz / (prescaler * desired interrupt freq. - 1) = 16000000 / (1024 * 100 - 1) = 156 (must be < 65536).
  TCCR1B |= (1 << WGM12);               //Turn on CTC mode.
  TCCR1B |= (1 << CS12) | (1 << CS10);  //Set CS10 and CS12 bits for 1024 prescaler.
  TIMSK1 |= (1 << OCIE1A);              //Enable timer compare interrupt.
  sei();  //Allow interrupts again.
}

/*
******************************************
* Arduino program main code to be looped *
******************************************/
void loop() {
  // put your main code here, to run repeatedly:
  displayValues();                                                                                      //Printing read out values from the greenhouse to display.
  moistureValue1 = moistureSensor1.moistureRead(moistureSensorPort1);                                   //Read moistureSensor1 value to check soil humidity.
  moistureValue2 = moistureSensor2.moistureRead(moistureSensorPort2);                                   //Read moistureSensor2 value to check soil humidity.
  moistureValue3 = moistureSensor3.moistureRead(moistureSensorPort3);                                   //Read moistureSensor3 value to check soil humidity.
  moistureValue4 = moistureSensor4.moistureRead(moistureSensorPort4);                                   //Read moistureSensor4 value to check soil humidity.
  moistureValueMean = moistureMeanValue(moistureValue1, moistureValue2, moistureValue3, moistureValue4);    //Mean value from all sensor readouts.
  tempValue = humiditySensor.readTemperature(false);                                                    //Read temperature value from DHT-sensor. "false" gives the value in °C.
  //humidValue = humiditySensor.readHumidity();                                                           //Read humidity value from DHT-sensor.
  tempThresholdCompare();
  lightRead();                                                                                          //Read light sensor UV value.
  ledLightStart();                                                                                      //Start LED strip lighting.
  //lightTimer(uvValue, lightValue);  NOT IN USE!!!
  pumpStart();                                                                                          //Start pump to pump water to plant.
  waterLevelRead();                                                                                     //Check water level in water tank.
  alarmMessageDisplay();                                                                                //Print alarm messages to display for any faults that is currently active. Warning messages on display will alert user to take action to solve a certain fault.
}

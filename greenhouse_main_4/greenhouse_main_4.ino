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
#define flowSensor 3
#define waterLevelSwitch 12
#define lightRelay 6
#define clockSetButton 7
#define clockModeButton 2

//Arduino UNO base shield layout
/*
################### ARDUINO UNO ############################
#|__________________DIGITAL_(PWM_~)_______________________|#
#||X|X|X|GND|13|12|~11|~10|~9| 8|<>| 7|~6|~5| 4|~3| 2|X|X||#
#|- - - - - - - - - - - - - - - - - - - - - - - - - - - - |#
#||A3|      |D4|     |D3|     |D2|     |UART|             |#
#|                                                        |#
#||A2|      |D8|     |D7|     |D6|     |D5|               |#
#|                                                        |#
#||A1|      |I2C|    |I2C|    |I2C|    |I2C|              |#
#|      - - - - - - - - - - - - - - - - - - - - - - - - - |#
#||A0| | |X|X|X|3.3V|5V|GND|GND|Vin|<>|A0|A1|A2|A3|A4|A5| |#
#|_____|________POWER_____________________ANALOG IN_______|#
################### ARDUINO UNO ############################

GROVE connector
A3:   Moisture sensor4
A2:   Moisture sensor3
A1:   Moisture sensor2
A0:   Moisture sensor1
D4:   Humidity and temperature sensor
D8:   Water pump relay
I2C:  OLED display
D3:   Water flow sensor
D7:   SET-button
I2C:  'EMPTY'
D2:   MODE-button
D6:   LED lighting relay
I2C:  Light sensor
UART: 'EMPTY'
D5:   'EMPTY'
I2C:  'EMPTY'

DIGITAL (PWM~)
GND:  10 kohm resistor in in series with with 12 (I/O).
12:   10 kohm resistor parallell with signal wire1 to water tank level switch. Resistor is in series with GND (I/O).
11~:  Signal wire1 to temperature rotary encoder.
10~:  Signal wire2 to temperature rotary encoder.

POWER
5V:   Supply wire to water tank level switch in parallell supply wire to temperature rotary encoder.
GND:  Ground wire to temperature rotary encoder.
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
int moistureMeanValue;                    //Mean value of all 4 moisture sensors.
bool moistureDry = false;                 //Activates warning message on display based on moisture mean value. 'true' if soil for mean value sensor is too dry.
bool moistureWet = false;                 //Activates warning message on display based on moisture mean value. 'true' if soil for mean value sensor is too wet.
int moistureThresholdLow = 300;
int moistureThresholdHigh = 700;

//Temperature and humidity sensor.
const uint8_t DHTTYPE = DHT11;            //DHT11 = Arduino UNO model is being used.
DHT humiditySensor(DHTPIN, DHTTYPE);      //Create humidity sensor from DHT class.
float tempValue;
//float humidValue;
bool tempValueFault = false;              //Indicate if read out temperature is higher than temperature treshold that has been set by adjusting temperature rotary encoder. Variable is 'false' when read out temperature is below set temperature threshold.

//Rotary encoder to adjust temperature threshold.
int tempThresholdValue = 60;              //Starting value for temperature threshold adjustment is 30°C.
int aLastState;

//Debouncing button press on button connected to external interrupt.
volatile unsigned long pressTimePrev;     //Variable to store previous millis() value.
int debounceTimePeriod = 300;             //Delay time before interrupt function is started.

//Light sensor.
SI114X lightSensor;                       //Light sensor object created.
uint16_t lightValue;                      //Light read out, probably presented in the unit, lux.
uint16_t uvValue;                       
//uint16_t irValue;                       //IR read out not in use.

//LED lighting.
bool ledLightState = false;               //Indicate current status of LED lighting. Variable is 'true' when LED lighting is turned on.
bool ledLightFault = false;               //Indicate if LED lighting is not turned on/not working when LED lighting has been turned on.

//Water pump and flow sensor.
volatile int rotations;
int waterFlowValue; 
bool pumpState = false;                   //Indicate current status of water pump. Variable is 'true' when water pump is running.
bool waterFlowFault = false;              //Indicate if water is being pumped when water pump is running. Variable is 'false' when water flow is above threshold value. 
int flowThresholdValue = 99;              //Variable value specifies the minimum water flow threshold required to avoid setting water flow fault.

//Water level switch.
bool waterLevelValue;                     //If variable is 'false' water level is OK. If 'true' tank water level is too low.

//Internal clock to keep track of current time.
int hourPointer1 = 0;
int hourPointer2 = 0;
int minutePointer1 = 0;                 //1-digit of minute pointer.
int minutePointer2 = 0;                 //10-digit of minute pointer.
int secondPointer1 = 0;                 //1-digit of second pointer.
int secondPointer2 = 0;                 //10-digit of second pointer.
bool pushButton1 = false;
bool minuteInputMode = false;
bool hourInputMode = true;
bool clockStartMode = false;
bool clockSetFinished = false;
int divider100 = 0;
int divider50 = 0;                          
bool flashClockPointer = false;         //Variable to create lash clock pointer when in "set clock" mode.       
int x = 0;                              //Toggle variable to flash current clock pointer.

//Alarm messages to display.
bool alarmMessageEnabled = false;       //Enable any alarm to be printed to display. If variable is 'true' alarm is enable to be printed to display.
unsigned long alarmTimePrev = 0;        //Used to read relative time 
unsigned long alarmTimePeriod = 2100;   //Variable value specifies in milliseconds, for how long time each warning message will be shown on display before cleared and/or replaced by next warning message.

//Toggle display modes.
bool startupImageDisplay = true;        //Any variable is set to 'true' when that screen mode is currently printed to display.
bool setTimeDisplay = false;                 
bool readoutValuesDisplay = false;
bool serviceModeDisplay = false;
bool clockViewFinished = false;
bool flowFaultDisplay = false;
int y = 0;                              //Toggle variable to clear/set water flow fault.

/*
---------------------
|Greenhouse program.|
--------------------*/
bool greenhouseProgramStart = false;        //If variable is set to 'true', automatic water and lighting control of greenhouse is turned on.

//Water pump.
bool waterPumpEnabled = false;              //Enable/Disable water pump to run.
unsigned int checkMoisturePeriod = 20000;        //Loop time, in milliseconds, for how often water pump is activated based upon measured soil moisture value.         
unsigned long checkMoistureStart = 0;
unsigned int checkWaterFlowPeriod = 2000;
unsigned long checkWaterFlowStart = 0;    
unsigned int waterPumpTimePeriod = 3000;  //Sets the time for how long water pump will run each time it is activated.
unsigned long waterPumpTimeStart = 0; 

//LED lighting.
int uvThresholdValue = 4;                   //UV threshold value for turning LED lighting on/off.
//int lightThresholdValue = 1500;           //Light threshold value (lux) for turning LED lighting on/off.
bool ledLightEnabled = false;               //Enable/Disable start of LED lighting.
unsigned int checkLightNeedPeriod = 5000;   //Loop time for how often measured light value is checked. This enables/disables start of LED lighting.
unsigned long checkLightNeedStart = 0;
unsigned int checkLightFaultPeriod = 4000;  //Delay time after LED lighting has been turned ON, before checking if it works.
unsigned long checkLightFaultStart = 0;
bool insideTimeIntervalLow = false;
bool insideTimeIntervalHigh = false;


/*
==============================================================
|| Bitmap image 1 to be printed on OLED display at startup. ||
============================================================== */
const unsigned char greenhouse[] PROGMEM= {
//Startup image 1.
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80, 0xE0, 0x78, 0x18, 0x0C, 0x04, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04, 0x00,
0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0x40,
0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0x40, 0xC0, 0xC0, 0x80, 0x00,
0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xC0, 0xC0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0x40, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x0F, 0x7F, 0xF0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x04, 0x06, 0x06, 0x06, 0xFE, 0xFE,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xFF, 0x89, 0x08, 0x08, 0x08,
0x08, 0x09, 0x0F, 0x0E, 0x00, 0x00, 0x7C, 0xFF, 0x89, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x0F,
0x0E, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFE, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFC, 0xFE,
0x7F, 0x7F, 0xBF, 0xDF, 0xF7, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x80,
0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0C, 0x18, 0x18, 0xF0,
0xE0, 0x00, 0x00, 0x7C, 0xFF, 0x89, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x0F, 0x0E, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01,
0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03,
0x83, 0x43, 0x43, 0xF0, 0x80, 0x40, 0x40, 0x80, 0xC1, 0x43, 0x43, 0xE3, 0x83, 0x43, 0x43, 0xC1,
0x00, 0xC0, 0x40, 0x80, 0x83, 0x43, 0x40, 0x80, 0x80, 0x40, 0x40, 0xF0, 0x03, 0x03, 0xF0, 0x40,
0x40, 0x83, 0xC3, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xE3, 0x63, 0x80, 0x00, 0x00, 0x01, 0xE4,
0x02, 0x03, 0x23, 0x23, 0x23, 0xE1, 0x21, 0x20, 0x20, 0x00, 0xE0, 0x20, 0x20, 0x20, 0x21, 0x03,
0xE3, 0xE3, 0x83, 0x01, 0x00, 0x03, 0xE3, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x02, 0x02, 0x03, 0x01, 0x03, 0x03, 0x01, 0x02, 0x03, 0x01, 0x03, 0x01, 0x0A, 0x0A, 0x07,
0x00, 0x03, 0x00, 0x03, 0x01, 0x03, 0x03, 0x01, 0x01, 0x02, 0x02, 0x03, 0x00, 0x00, 0x03, 0x02,
0x02, 0x01, 0x04, 0x03, 0x00, 0x00, 0x00, 0x38, 0x0F, 0x05, 0x04, 0x07, 0x1C, 0x20, 0x00, 0x3F,
0x20, 0x20, 0x20, 0x20, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x22, 0x22, 0x22, 0x22, 0x00,
0x3F, 0x00, 0x01, 0x07, 0x0C, 0x38, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
==============================================================
|| Bitmap image 2 to be printed on OLED display at startup. ||
============================================================== */
const unsigned char features[] PROGMEM= {
//Startup image 2.
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x70, 0x1C, 0x0E, 0x70, 0x3C, 0x06, 0x64,
0x54, 0x78, 0x00, 0x7E, 0x44, 0x00, 0x38, 0x54, 0x54, 0x18, 0x7C, 0x04, 0x00, 0x00, 0x00, 0x7E,
0x40, 0x40, 0x78, 0x54, 0x54, 0x58, 0x00, 0x1C, 0x70, 0x38, 0x04, 0x38, 0x54, 0x54, 0x18, 0x7E,
0x00, 0x00, 0x00, 0x4C, 0x4A, 0x52, 0x32, 0x1C, 0x70, 0x1C, 0x38, 0x70, 0x0C, 0x7E, 0x00, 0x04,
0x7E, 0x44, 0x38, 0x44, 0x44, 0x00, 0x7E, 0x04, 0x04, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0x5E, 0x46, 0x78, 0xC0, 0x00, 0x00, 0xF8,
0x10, 0x08, 0x00, 0xF0, 0x18, 0x08, 0x08, 0xFE, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xF8,
0x00, 0xFA, 0x00, 0x00, 0xF8, 0x10, 0x08, 0x08, 0x18, 0xF0, 0x00, 0xE0, 0x10, 0x08, 0x08, 0x10,
0xE0, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0xFE, 0x0E, 0x18,
0x70, 0xC0, 0x80, 0xFE, 0x00, 0x00, 0xF8, 0x04, 0x02, 0x02, 0x02, 0x04, 0xF8, 0x00, 0x00, 0x00,
0x7E, 0x40, 0x40, 0x00, 0x7E, 0x38, 0x44, 0x44, 0xFC, 0x00, 0x7E, 0x04, 0x04, 0x78, 0x00, 0x7E,
0x44, 0x00, 0x00, 0x00, 0x4C, 0x4A, 0x32, 0x00, 0x38, 0x54, 0x54, 0x18, 0x7C, 0x04, 0x04, 0x78,
0x00, 0x48, 0x54, 0x20, 0x38, 0x44, 0x44, 0x38, 0x00, 0x7C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x03,
0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x02, 0x03, 0x00, 0x00, 0x01, 0x03, 0x02, 0x02, 0x09, 0x0B,
0xF8, 0x0B, 0xC8, 0xA0, 0xA3, 0xA0, 0xC0, 0xE0, 0x20, 0x23, 0xE0, 0x20, 0x21, 0xC2, 0x02, 0xE1,
0x20, 0x20, 0x20, 0xC0, 0x00, 0xC0, 0xA1, 0xA2, 0xC2, 0x02, 0xE1, 0x20, 0x20, 0xA3, 0xA0, 0xC0,
0x00, 0x20, 0xF3, 0x23, 0xE0, 0x00, 0x00, 0xE1, 0x02, 0x02, 0xE2, 0x21, 0x00, 0xC0, 0xA0, 0xA0,
0xC0, 0x00, 0x00, 0x40, 0xA0, 0xA0, 0x21, 0xC1, 0xA0, 0xA0, 0xC0, 0x00, 0xE0, 0x20, 0x20, 0xC0,
0x00, 0x40, 0xA0, 0xA0, 0x20, 0xC0, 0x20, 0x20, 0x20, 0xC0, 0x00, 0xE0, 0x20, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00, 0x80, 0xE0, 0xE0, 0x80, 0x40, 0x40,
0x40, 0x80, 0xE0, 0x00, 0x80, 0x40, 0x40, 0x40, 0xE0, 0x40, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0xC0,
0x43, 0x00, 0x81, 0x42, 0x42, 0x82, 0x02, 0x03, 0xC0, 0xA0, 0x23, 0x20, 0x80, 0x43, 0x40, 0x8F,
0x02, 0xC2, 0x42, 0x41, 0x80, 0x81, 0x42, 0x42, 0x82, 0x40, 0x43, 0x40, 0x83, 0xC2, 0x42, 0x03,
0x00, 0x00, 0xF3, 0x1A, 0x09, 0x0A, 0x1A, 0xF3, 0x00, 0x00, 0xFB, 0x00, 0x00, 0x01, 0x02, 0xFA,
0x8A, 0x88, 0x88, 0x02, 0xFA, 0x0A, 0x09, 0x09, 0x12, 0xE2, 0x02, 0x00, 0x03, 0xF8, 0x08, 0x0B,
0x08, 0x12, 0xF2, 0x02, 0xE9, 0x01, 0x02, 0xE2, 0xA2, 0x21, 0x00, 0xE3, 0x20, 0x20, 0x60, 0xC0,
0x00, 0xF8, 0x00, 0x00, 0x20, 0x20, 0xC0, 0x00, 0x20, 0xC0, 0x00, 0xC0, 0x60, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x03, 0x00, 0x07, 0x03, 0x04, 0x04,
0x04, 0x03, 0x07, 0x00, 0x04, 0x05, 0x03, 0x00, 0x07, 0x34, 0xE0, 0x07, 0xC4, 0x77, 0xC0, 0x07,
0xE0, 0x30, 0x43, 0x45, 0xC5, 0x81, 0x40, 0xE0, 0x44, 0x44, 0x05, 0xC3, 0x43, 0xC5, 0x85, 0x05,
0xC0, 0x47, 0x40, 0x00, 0x07, 0x04, 0xF5, 0x13, 0x13, 0xE4, 0x04, 0xC4, 0x03, 0x07, 0xC0, 0x00,
0x00, 0xC0, 0x43, 0x46, 0xC4, 0x44, 0x46, 0xC3, 0x00, 0x00, 0xC7, 0x44, 0x44, 0xC4, 0x80, 0x07,
0x04, 0x04, 0x04, 0x00, 0x07, 0x04, 0x04, 0x04, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x04, 0x04,
0x04, 0x02, 0x01, 0x00, 0x07, 0x00, 0x00, 0x04, 0x05, 0x07, 0x00, 0x1F, 0x04, 0x04, 0x06, 0x03,
0x00, 0x07, 0x00, 0x06, 0x05, 0x05, 0x07, 0x00, 0x00, 0x09, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xC0, 0x80, 0x00, 0xC0, 0x00, 0x80, 0x80, 0x00, 0x80, 0xC0,
0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0xC3, 0x4E, 0x43, 0x00, 0xC7, 0x0E,
0x03, 0x80, 0x8C, 0x0A, 0x8A, 0x0F, 0x00, 0x8F, 0x08, 0x80, 0x07, 0x0A, 0x0A, 0x8A, 0x43, 0x40,
0x0F, 0x00, 0x80, 0x80, 0x00, 0x80, 0x8F, 0x81, 0x01, 0x00, 0x00, 0x8F, 0x08, 0x08, 0x8F, 0x80,
0x00, 0x0F, 0x80, 0x80, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x3F, 0x08, 0x08, 0x0C, 0x07, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0x0E, 0x01, 0x07, 0x0E, 0x01, 0x04, 0x0A, 0x0A, 0x0F, 0x00, 0x0F,
0x08, 0x07, 0x0A, 0x0A, 0x03, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x02, 0x02, 0x00, 0x0F, 0x00,
0x07, 0x08, 0x08, 0x07, 0x01, 0x0E, 0x07, 0x03, 0x0E, 0x03, 0x00, 0x00, 0x00, 0xE9, 0x09, 0x06,
0x00, 0x07, 0xEA, 0x2A, 0x23, 0x2F, 0x00, 0xE0, 0x2F, 0x20, 0x29, 0x4A, 0x84, 0x07, 0x08, 0x08,
0xE7, 0x00, 0x0F, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0xE0, 0x80,
0x80, 0x00, 0x00, 0x80, 0xC0, 0x80, 0x00, 0xA0, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00,
0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10,
0x10, 0x00, 0x1F, 0x12, 0x12, 0x12, 0x00, 0x1F, 0x10, 0x10, 0x10, 0x0C, 0x07, 0x00, 0x00, 0x00,
0x1F, 0x10, 0x10, 0x10, 0x00, 0x1F, 0x00, 0x0F, 0x59, 0x50, 0x50, 0x3F, 0x00, 0x00, 0x1F, 0x00,
0x00, 0x1F, 0x00, 0x00, 0x1F, 0x10, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x0F,
0x59, 0x50, 0x79, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
======================================================
|| Initialize OLED display and show startup images. ||
====================================================== */
void viewStartupImage() {
  if(startupImageDisplay) {
    Serial.println("startupImageDisplay");
    Wire.begin();
    SeeedOled.init();
    SeeedOled.clearDisplay();                         //Clear display.
    SeeedOled.setHorizontalMode();
    SeeedOled.setNormalDisplay();                     //Set display to normal mode (non-inverse mode).
    SeeedOled.setPageMode();                          //Set addressing mode to Page Mode.

    /*
    //Startup image 1.
    SeeedOled.drawBitmap(greenhouse, (128*64)/8);   //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
    delay(4000);                                    //Image shown for 4 seconds.
    SeeedOled.clearDisplay();                       //Clear the display.

    //Startup image 2.
    SeeedOled.drawBitmap(features, (128*64)/8);       //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
    delay(3000);                                      //Image shown for 3 seconds.
    SeeedOled.clearDisplay();                         //Clear the display.
    */
  
    startupImageDisplay = false;                      //Clear current screen display state.
    setTimeDisplay = true;                            //Set next display mode to be printed to display.  
    hourInputMode = true;                             //Set state in next display mode.
  }
}

/*
=========================================================================
|| VALUE READ OUT DISPLAY MODE. Print read out values to OLED display. ||
========================================================================= */
void viewReadoutValues() {
  //Clear redundant value digits from previous read out for all sensor values.
  SeeedOled.setTextXY(0, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(1, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(2, 42);
  SeeedOled.putString("      ");    
  SeeedOled.setTextXY(3, 39);
  SeeedOled.putString("         ");
  SeeedOled.setTextXY(4, 42);
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(4, 44);               //Clear symbols from previous display mode.
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
  SeeedOled.putNumber(moistureMeanValue);   //Print mean moisture value to display.

  //Select which text string to be printed to display depending of soil moisture.
  SeeedOled.setTextXY(0, 45);
  if(moistureDry == true && moistureWet == false) {
    SeeedOled.putString("Dry");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == false) {
    SeeedOled.putString(" OK");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == true) {
    SeeedOled.putString("Wet");             //Print string to display.
  }  

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
  SeeedOled.putNumber(waterFlowValue);      //Print water flow value to display.
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
===========================
|| Turn ON LED lighting. ||
=========================== */
void ledLightStart() {
  digitalWrite(lightRelay, HIGH);                                 //Turn on LED lighting.
  ledLightState = true;                                           //Update current LED lighting state, 'true' means lighting is on.
  Serial.println("LED lighting ON");
}

/*
=======================================
|| Check if LED lighting is working. ||
======================================= */
  //Alarm if light read out value does not get above light threshold when LED lighting is turned on.
void ledLightCheck() {  
  if(ledLightState == true && uvValue < uvThresholdValue) {        
    ledLightFault = true;                                       //If read out light value does not get above light threshold (lower light limit), fault variable is set to 'true' to alert user.
  }
  else {
    ledLightFault = false;
  }
  Serial.println("Check LED lighting fault");
}

/*
============================
|| Turn OFF LED lighting. ||
============================ */
void ledLightStop() {  
  digitalWrite(lightRelay, LOW);                                //Turn off LED lighting.
  ledLightState = false;                                        //Update current LED lighting state, 'false' means lighting is off.
  Serial.println("LED lighting OFF");
}

/*
======================================================================================
|| Check current clock time and light need to enable/disable start of LED lighting. ||
====================================================================================== */
void checkLightNeed() {
  //Check if current time is within specified time interval: 06:?? - 23:??.
  //Check if current time is above lower time boundary.
  if(hourPointer2 >= 0) {                   //Decode of 10-digit hour pointer.
    if(hourPointer1 >= 6) {                 //Decode of 1-digit hour pointer.                  
      insideTimeIntervalLow = true;         //Current clock time is above lower time boundary where LED lighting is allowed.
    }
  }
  
  //Check if current time is below upper time boundary.
  if(insideTimeIntervalLow == true) {       
    if(hourPointer2 >= 0) {
      if(hourPointer1 <=9) {                       
        insideTimeIntervalHigh = true;      //Current clock time is below upper time boundary where LED lighting is allowed.
      }
    }
  }
  //THIS PART OF CODE SUCKS BUT I LEAVE IT FOR NOW!!!!!!!!
  
  //Check if measured light value is below light threshold value.
  if(insideTimeIntervalLow == true && insideTimeIntervalHigh == true) {
    if(uvValue < uvThresholdValue) {
      ledLightEnabled = true;
    }
    else {
      ledLightEnabled = false;
    }
  }
  Serial.println("Check light need");
}

/*
==============================
|| Read water level switch. ||
============================== */
void waterLevelRead() {
  waterLevelValue = digitalRead(waterLevelSwitch);                  //If variable is 'false' water level is OK. If 'true' tank water level is too low.
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
=======================================================================================================
|| Start water pump, read out water flow sensor. ||
===================================================*/
void waterPumpStart() {                
  digitalWrite(pumpRelay, HIGH);          //Start water pump.
  pumpState = true;                       //Update current water pump state, 'true' means water pump is running.
  Serial.println("Water pump ON");

  //Calculate water flow (Liter/hour) by counting number of rotations that flow sensor makes. Water flow sensor is connected to interrupt pin.
  if(pumpState == true) {                 //Only check water flow when water pump is running.
    rotations = 0;                        
    delay(1000);                          //Count number of rotations during one second to calculate water flow in Liter/hour. 
    waterFlowValue = (rotations * 60) / 7.5;   //Calculate the flow rate in Liter/hour.
  }
  else {
    waterFlowValue = 0;                   //Clearing water flow value when pump is not running to prevent any faulty value from water flow sensor.
  }
}

/*
======================
|| Stop water pump. ||
====================== */
void waterPumpStop() {
  digitalWrite(pumpRelay, LOW);         //Stop water pump.
  pumpState = false;                    //Update current water pump state, 'false' means water pump not running.
  Serial.println("Water pump OFF");
}

/*
========================================================================
|| Check if water flow is above a certain level when pump is running. ||
======================================================================== */
void waterFlowCheck() {
  if(waterFlowValue < flowThresholdValue) {  //Check current water flow.
    waterFlowFault = true;              //Set fault code.  
  }
  else {
  waterFlowFault = false;               //Clear fault code.
  }
  Serial.println("Check water flow");
}

/*
======================================
|| Enable/Disable water pump start. ||
====================================== */
void checkWaterNeed() {
  //Water pump is enabled if soil moisture is too dry or at the same time as no water related fault codes are set.
  if(moistureDry == true && moistureWet == false) {            
    if(waterLevelValue == false && waterFlowFault == false) {   //Make sure no water related fault codes are set.
      waterPumpEnabled = true;                                  //Enable water pump to run let it start when activated.
    }
    else {
      waterPumpEnabled = false;                                 //Disable water pump to prevent it from starting.  
    }
  }
  Serial.println("Check water need");
}

/*
=============================================================================================================
|| Timer interrupt to read temperature threshold value, that is being adjusted by rotary encoder.          ||
|| Timer interrupt is also used to work as a second ticker for the built clock, included in this function. ||
============================================================================================================= */
ISR(TIMER1_COMPA_vect) {  //Timer interrupt 100Hz to read temperature threshold value set by rotary encoder.

  /*************************************
  |Temperature rotary encoder read out.|
  **************************************/
  //Reading preset temperature threshold thas is being adjusted by rotary encoder knob.
  int minTemp = 28;   //Temperature value can be set within the boundaries of 14 - 40°C (minTemp - maxTemp). Temp value is doubled to reduce rotary sensitivity and increase knob rotation precision.
  int maxTemp = 80;
  int aState;

  aState = digitalRead(rotaryEncoderOutpA);                         //Reads the current state of the rotary knob, outputA.
  
  if(aState != aLastState) {                                        //A turn on rotary knob is detected by comparing previous and current state of outputA.
    if(digitalRead(rotaryEncoderOutpB) != aState && tempThresholdValue <= maxTemp) { //If outputB state is different to outputA state, that meant the encoder knob is rotation clockwise.
      tempThresholdValue++;                                         //Clockwise rotation means increasing position value. Position value is only increased if less than max value.
    }
    else if(tempThresholdValue > minTemp) {
      tempThresholdValue--;                                         //Counter clockwise rotation means decreasing position value.
    }
  }
  aLastState = aState;                                              //Updates the previous state of outputA with current state.

  /***************************************************
  |Internal clock used to keep track of current time.|
  ****************************************************/
  //Internal clock.
  divider50++;                        
    if(divider50 == 50) {               //Gives 2Hz pulse to feed the flashing of pointer digits when in "set mode".
    divider50 = 0;                      //Reset divider variable.
    x++;
    if(x == 1) {
      flashClockPointer = true;
    }
    else {
      flashClockPointer = false;
      x = 0;
    }
  } 
  
  if(clockStartMode == true) {          //This function runs in a frequency of 100Hz. To the second pointer tick in 1Hz frequency this variable as a divider.
    divider100++;                       
      
    if(divider100 == 100) {             //Gives a 1Hz pulse to feed the second pointer."
      //This function will be run every second, 1Hz and therefore it will work as second pointer that increases its value/ticks every second.
      divider100 = 0;                   //Reset divider variable.
      secondPointer1++;                 //Increase second pointer every time this function runs.
      
      //Second pointer.
      if(secondPointer1 == 10) {        //If 1-digit second pointer reaches a value of 10 (elapsed time is 10 seconds).
        secondPointer2++;               //Increase 10-digit second pointer.
        secondPointer1 = 0;             //Clear 1-digit pointer.
      }
      if(secondPointer2 == 6) {         //If 10-digit pointer reaches a value of 6 (elapsed time is 60 seconds).
        minutePointer1++;               //Increase minute pointer.
        secondPointer2 = 0;             //Clear 10-digit second pointer.
      }
      //Minute pointer.
      if(minutePointer1 == 10) {        //If 1-digit minute pointer reaches a value of 10 (elapsed time is 10 minutes).
        minutePointer2++;               //Increase 10-digit minute pointer.
        minutePointer1 = 0;             //Clear 1-digit minute pointer.
      }
      if(minutePointer2 == 6) {         //If 10-digit minute pointer reaches a value of 6 (elapsed time is 60 minutes).
        hourPointer1++;                 //Increase 1-digit hour pointer.
        minutePointer2 = 0;              //Clear 10-digit minute pointer.
      }
      //Hour pointer.
      if(hourPointer1 == 10) {          //If 1-digit hour pointer reaches a value of 10 (elapsed time is 10 hours).
        hourPointer2++;                 //Increase 10-digit hour pointer.
        hourPointer1 = 0;               //Clear 1-digit hour pointer.
      }
      if(hourPointer2 == 2 && hourPointer1 == 4) {  //If 1-digit and 10-digit hourPointer combined reaches 24 (elapsed time is 24 hours).
        hourPointer1 = 0;                           //Clear both hour digits.
        hourPointer2 = 0;
      }
    }
  }
}

/*
===============================================================
|| Set current time by using SET- and MODE-buttons as input. ||
=============================================================== */
void setClockTime() {
  if(pushButton1 == true && hourInputMode == true) {
    delay(170);                                                 //Delay to avoid contact bounce.
    hourPointer1++;                                             //Increase hour pointer every time button is pressed.
    if(hourPointer1 == 10) {                                    //If 1-digit hour pointer reaches 10, increase 10-digit hour poínter.
      hourPointer2++;
      hourPointer1 = 0;
    }
    if(hourPointer2 == 2 && hourPointer1 == 4) {                //If 10-digit hour pointer reaches a value of 2 and 1-digit hour pointer reaches a value of 4 (elapsed time is 24 hours).
      hourPointer2 = 0;                                         //Clear both hour pointer values.
      hourPointer1 = 0;
    }
  }
  else if(pushButton1 == true && minuteInputMode == true) {  
    delay(170);
    minutePointer1 += 5;                                        //Increase minute pointer with 5 minutes every time button is pressed.
    if(minutePointer1 == 10) {                                  //If 1-digit minute pointer reaches a value of 10, increase 10-digit minute pointer.
      minutePointer2++;
      minutePointer1 = 0;
    }
    if(minutePointer2 == 6) {                                   //If 10-digit minute pointer reaches a value of 6 (elapsed time is 60 minutes).
      minutePointer2 = 0;                                       //Clear 10-digit minute pointer.
    }
  }
}

/*
======================
|| Reset clock time ||
====================== */
void resetClockTime() {
  //Stop clock and reset all clock pointers.
  clockStartMode = false;                                       //Stop clock from ticking.
  hourPointer1 = 0;
  hourPointer2 = 0;
  minutePointer1 = 0;                 
  minutePointer2 = 0;                 
  secondPointer1 = 0;                 
  secondPointer2 = 0;         
}

/*
======================================================================================
|| Toggle set modes and screen display modes when clockModeButton is being pressed. ||
====================================================================================== */
void toggleDisplayMode() {
  //Debouncing button press to avoid multiple interrupts, display toggles.
  if((millis() - pressTimePrev) >= debounceTimePeriod) {

    //Toggle display modes every time MODE-button is pressed.
    if(setTimeDisplay == true) {
      Serial.println("setTimeDisplay");
      if(hourInputMode == true) {
        hourInputMode = false;                  //Hour pointers have been set.
        minuteInputMode = true;                 //Continue with setting minute pointers.
        Serial.println("hourInputMode");
      }
      else if(minuteInputMode == true) {
        minuteInputMode = false;                //Minute pointers have been set.
        clockStartMode = true;                  //Start clock. Clock starts ticking.
        clockSetFinished = true;
        Serial.println("minuteInputMode");
      }
      else if(clockSetFinished == true) {
        clockSetFinished = false;               //Clear current state in display mode.
        setTimeDisplay = false;                 //Clear current display mode.
        readoutValuesDisplay = true;            //Set next display mode to be printed to display.
        alarmMessageEnabled = true;             //Enable any alarm message to be printed to display.
        greenhouseProgramStart = true;          //Start greenhouse program.
        Serial.println("clockSetFinished");
      }
    }
    else if(readoutValuesDisplay == true) {
      readoutValuesDisplay = false;               //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      alarmMessageEnabled = false;                //Disable any alarm message from being printed to display.
      //SeeedOled.clearDisplay();                   //Clear display.
      serviceModeDisplay = true;                  //Set next display mode to be printed to display.
      Serial.println("readoutValuesDisplay");
    }
    else if(serviceModeDisplay == true) {
      serviceModeDisplay = false;                 //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedOled.clearDisplay();                   //Clear display.
      readoutValuesDisplay = true;                //Set next display mode to be printed to display.
      alarmMessageEnabled = true;                 //Enable any alarm message from being printed to display.
      Serial.println("serviceModeDisplay");
    }
    else if(flowFaultDisplay == true) {
      flowFaultDisplay = false;                   //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedOled.clearDisplay();                   //Clear display.
      Serial.println("flowFaultDisplay");
      
      //Check if water flow fault code has been cleared by user or not.
      if(waterFlowFault == false) {               //Fault code has been cleared by user. Continue to run greenhouse program.               
        greenhouseProgramStart = true;            
        readoutValuesDisplay = true;              //Set next display mode to be printed to display.
        alarmMessageEnabled = true;               //Enable any alarm message from being printed to display.
        Serial.println("Resume program"); 
      }
      else {                                      //If flow fault code is not cleared, reboot greenhouse program.
        waterFlowFault = false;                   //Clear water flow fault code.
        resetClockTime();                         //Reset clock time.
        startupImageDisplay = true;               //Reboot greenhouse program by printing the startup image to display.                
        Serial.println("Go to startupImageDisplay");             
      }
    }

    //Check if water flow fault code is active. If active enter flow fault display to handle fault code.
    if(waterFlowFault == true) {
      readoutValuesDisplay = false;               //Clear any of current screen display modes to enable next display mode to shown next time MODE-button is pressed.
      serviceModeDisplay = false;
      alarmMessageEnabled = false;
      flowFaultDisplay = true;                    //Set next display mode to be printed to display.
      greenhouseProgramStart = false;             //Stop greenhouse program.
    }
  pressTimePrev = millis();
  }
}

/*
===================================================================================================
|| SET CLOCK TIME DISPLAY MODE. Print clock values to OLED display to let user set current time. ||
=================================================================================================== */
void setClockDisplay() {
  SeeedOled.setTextXY(0, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Set current time");              //Print text to display.
  SeeedOled.setTextXY(1, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Use buttons:");                  
  SeeedOled.setTextXY(2, 0);                            
  SeeedOled.putString("SET = inc. p.val");              
  SeeedOled.setTextXY(3, 0);                            
  SeeedOled.putString("MODE = h or min");               
  SeeedOled.setTextXY(5, 20);                           
  SeeedOled.putString("HH");                            
  SeeedOled.setTextXY(5, 23);                           
  SeeedOled.putString("MM");                            
  SeeedOled.setTextXY(5, 26);                           
  SeeedOled.putString("SS");                            

  if(flashClockPointer == true && hourInputMode == true) {
    SeeedOled.setTextXY(6, 20);                           
    SeeedOled.putString("  ");
  }
  else {
    SeeedOled.setTextXY(6, 20);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
    SeeedOled.setTextXY(6, 21);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  }
  
  SeeedOled.setTextXY(6, 22);                             //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                               //Print text to display.
  
  if(flashClockPointer == true && minuteInputMode == true) {
    SeeedOled.setTextXY(6, 23);                           
    SeeedOled.putString("  ");
  }
  else {
    SeeedOled.setTextXY(6, 23);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(minutePointer2);                  //Print 10-digit hour pointer value to display.
    SeeedOled.setTextXY(6, 24);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(minutePointer1);                  //Print 1-digit hour pointer value to display.
  }

  SeeedOled.setTextXY(6, 25);                           
  SeeedOled.putString(":");                             
  SeeedOled.setTextXY(6, 26);                           
  SeeedOled.putNumber(secondPointer2);                   //Print second digit of second pointer value to display.
  SeeedOled.setTextXY(6, 27);                           
  SeeedOled.putNumber(secondPointer1);                   //Print first digit of second pointer value to display.
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
============================================================================================================
|| ALARM MESSAGE TO DISPLAY. Print alarm message to OLED display for any fault that is currently active . ||
============================================================================================================ */
void alarmMessageDisplay() {       
  unsigned long alarmTimeNow;
  unsigned long alarmTimeDiff;                            //Calculate time difference from when the warning message function was started. Keeps track for how long time each warning message is shown on display.
                                                              
  //Print multiple warning messages to display, using the same space of display. One alarm message after another.
  alarmTimeNow = millis();                                //Read millis() value to get the time stamp when first alarm message was printed to display.                      
  alarmTimeDiff = alarmTimeNow - alarmTimePrev;
  //Serial.print("alarmTimeDiff: ");
  //Serial.println(alarmTimeDiff);

  if(alarmMessageEnabled == true) {                       //Any alarm can only be printed to display if variable is set to 'true'.
    /******************
    |Water flow fault.|
    *******************/
    if(alarmTimeDiff <= alarmTimePeriod) {
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
    if(alarmTimePeriod < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 2) {
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
    if(alarmTimePeriod * 2 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 3) { 
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
    if(alarmTimePeriod * 3 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 4) { 
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
  
    if(alarmTimePeriod * 4 < alarmTimeDiff) {
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                        ");    //Clear row to enable other warnings to be printed to display.
      alarmTimePrev = millis();                           //Read millis() value to reset time delay calculation.
    }  
  }
}

/*
===========================================================================
|| SERVICE MODE DISPLAY MODE. Print service mode screen to OLED display. ||
=========================================================================== */
void viewServiceMode() {
  //Clear redundant value digits from previous read out for all sensor values.
  SeeedOled.setTextXY(0, 39);
  SeeedOled.putString(" ");
  SeeedOled.setTextXY(1, 41);
  SeeedOled.putString("   ");  
  SeeedOled.setTextXY(2, 19);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(2, 28);
  SeeedOled.putString("   ");     
  SeeedOled.setTextXY(3, 19);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(3, 28);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(4, 27);
  SeeedOled.putString(" ");
  SeeedOled.setTextXY(5, 26);
  SeeedOled.putString("  ");
  SeeedOled.setTextXY(5, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(6, 42);
  SeeedOled.putString("  ");
  SeeedOled.setTextXY(6, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(7, 45);
  SeeedOled.putString("  ");
  
  //Display clock.
  SeeedOled.setTextXY(0, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Clock: ");                       //Print string to display.
  //Hour pointer.
  SeeedOled.setTextXY(0, 40);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  SeeedOled.setTextXY(0, 41);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  
  SeeedOled.setTextXY(0, 42);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Minute pointer.
  SeeedOled.setTextXY(0, 43);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(minutePointer2);                  //Print 10-digit hour pointer value to display.
  SeeedOled.setTextXY(0, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(minutePointer1);                  //Print 1-digit hour pointer value to display.

  SeeedOled.setTextXY(0, 45);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Second pointer.
  SeeedOled.setTextXY(0, 46);                           
  SeeedOled.putNumber(secondPointer2);                  //Print second digit of second pointer value to display.
  SeeedOled.setTextXY(0, 47);                           
  SeeedOled.putNumber(secondPointer1);                  //Print first digit of second pointer value to display.

  //Display moisture sensor values.
  SeeedOled.setTextXY(1, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Moisture:");                     //Print text to display.
  SeeedOled.setTextXY(2, 0);                            
  SeeedOled.putString("S1[");                           //Print text to display.
  SeeedOled.setTextXY(2, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue1);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(2, 22);                            
  SeeedOled.putString("],");                     

  SeeedOled.setTextXY(2, 25);                            
  SeeedOled.putString("S2[");                           //Print text to display.
  SeeedOled.setTextXY(2, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue2);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(2, 31);                            
  SeeedOled.putString("]");
  
  SeeedOled.setTextXY(3, 0);                            
  SeeedOled.putString("S3[");                           //Print text to display.
  SeeedOled.setTextXY(3, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue3);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(3, 22);                            
  SeeedOled.putString("],");                     

  SeeedOled.setTextXY(3, 25);                            
  SeeedOled.putString("S4[");                           //Print text to display.
  SeeedOled.setTextXY(3, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue4);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(3, 31);                            
  SeeedOled.putString("]");

  //Alarm messsage status.
  SeeedOled.setTextXY(4, 0);                            
  SeeedOled.putString("tempValue: ");                   //Print text to display.
  SeeedOled.setTextXY(4, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(tempValueFault);                  //Print moisture sensor1 value.

  SeeedOled.setTextXY(5, 0);                            
  SeeedOled.putString("ledLight: ");                    //Print text to display.
  SeeedOled.setTextXY(5, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(ledLightFault);                   //Print moisture sensor1 value.

  SeeedOled.setTextXY(6, 0);                            
  SeeedOled.putString("waterFlow: ");                   //Print text to display.
  SeeedOled.setTextXY(6, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(waterFlowFault);                  //Print moisture sensor1 value.

  SeeedOled.setTextXY(7, 0);                            
  SeeedOled.putString("waterLevel: ");                  //Print text to display.
  SeeedOled.setTextXY(7, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(waterLevelValue);                 //Print moisture sensor1 value.
}

/*
==========================================================================================
|| Calculate moisture mean value from moisture measurements and evaluate soil humidity. ||
========================================================================================== */
int calculateMoistureMean(int moistureValue1, int moistureValue2, int moistureValue3, int moistureValue4) {
  int moistureValues[4] = {moistureValue1, moistureValue2, moistureValue3, moistureValue4};
  int moistureMax = 0;                                      //Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the highest moisture value.
  int moistureMin = moistureValues[0];                      //First value in array of values used as reference value. Variable used to store a moisture value when comparing it to other moisture sensor values and finally store the lowest moisture value.
  int maxIndex;                                             //Index in array for max moisture value.
  int minIndex;                                             //Index in array for min moisture value.
  int moistureSum = 0;
  int moistureMean;                                         //Stores the moisture mean value before returned to main program.
  
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

  //Remove maximum and minimum moisture values from moisture array.
  moistureValues[minIndex] = 0;                             
  moistureValues[maxIndex] = 0;                             

  for(int i=0; i<sizeof(moistureValues)/sizeof(int); i++) {
    moistureSum += moistureValues[i];                       //Sum the remaining moisture sensor values.
  }
  moistureMean = moistureSum / 2;                           //Calculate mean moisture value with max and min values excluded.

  //Evaluate soil humidity based on moisture mean value.
  if(moistureMean <= moistureThresholdLow) {                //Soil humidity is too low.
    moistureDry = true;                                     //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureWet = false;
  }
  else if(moistureMean > moistureThresholdLow && moistureMean <= moistureThresholdHigh) { //Soil humidity is good.
    moistureWet = false;                                    //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureDry = false;
  }
  else if(moistureMean > moistureThresholdHigh) {           //Soil humidity is to high.
    moistureWet = true;                                     //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureDry = false;
  }
  return moistureMean;
}
/*
=========================================================================
|| FLOW FAULT DISPLAY MODE. Print service mode screen to OLED display. ||
========================================================================= */
void resolveFlowFault() {
  //Clear redundant symbols from previous screen mode.
  SeeedOled.setTextXY(0, 28);
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(2, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(5, 45);
  SeeedOled.putString("   ");
  
  //Print fault code instruction to display. To let user resolve fault.
  SeeedOled.setTextXY(0, 0);
  SeeedOled.putString("waterFlow: ");
  SeeedOled.setTextXY(0, 27);
  SeeedOled.putNumber(waterFlowFault);
  
  SeeedOled.setTextXY(1, 0);
  SeeedOled.putString("----------------");
  SeeedOled.setTextXY(2, 0);
  SeeedOled.putString("Chk hardware: ");
  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString("*is hose empty?");
  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString("*vacuum in tank?");
  SeeedOled.setTextXY(5, 0);
  SeeedOled.putString("*Done? Press:");
  SeeedOled.setTextXY(6, 0);
  SeeedOled.putString("SET = clear/set");
  SeeedOled.setTextXY(7, 0);
  SeeedOled.putString("MODE = confirm");

  //Clear fault code status.
  if(pushButton1 == true && y == 0) {
    waterFlowFault = false;           //Clear fault code to let water pump run.
    y = 1;
    Serial.println("waterFlowFault active");
  }
  else if(pushButton1 == true && y == 1) {
    waterFlowFault = true;            //Let fault code stay active.
    y = 0;
    Serial.println("waterFlowFault inactive");
  }
}

/*
*******************************
* Arduino program setup code. *
*******************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(moistureSensorPort1, INPUT);
  pinMode(moistureSensorPort2, INPUT);
  pinMode(moistureSensorPort3, INPUT);
  pinMode(moistureSensorPort4, INPUT);
  
  pinMode(flowSensor, INPUT);
  attachInterrupt(1, flowCount, RISING);  //Initialize interrupt to enable water flow sensor to calculate water flow pumped by water pump.

  attachInterrupt(0, toggleDisplayMode, RISING); //Initialize interrupt to toggle set modes when in clock set mode or toggling screen display mode when greenhouse program is running. Interrupt is triggered by clockModeButton being pressed.
  
  pinMode(pumpRelay, OUTPUT);
  //pinMode(pumpButton, INPUT);
  
  pinMode(waterLevelSwitch, INPUT);
  
  pinMode(lightRelay, OUTPUT);
  
  pinMode(rotaryEncoderOutpA, INPUT);
  pinMode(rotaryEncoderOutpB, INPUT);
  aLastState = digitalRead(rotaryEncoderOutpA);      //Read initial position value.

  pinMode(clockSetButton, INPUT);
  pinMode(clockModeButton, INPUT);
  
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
  
  //Set current time and toggle between different screen display modes.
  pushButton1 = digitalRead(clockSetButton);                        //Check if SET-button is being pressed.

  //Different functions to be run depending of which screen display mode that is currently active.
  if(startupImageDisplay == true) {
    viewStartupImage();                                             //Initialize the OLED Display and show startup images.
  }
  else if(setTimeDisplay == true) {                                 //Display time set screen only if current time has not been set.
    setClockTime();
    setClockDisplay();
  }
  else if(readoutValuesDisplay == true) {                           //Only display read out values after current time on internal clock, has been set.
    viewReadoutValues();                                            //Print read out values from the greenhouse to display.
  }
  else if(serviceModeDisplay == true) {
    viewServiceMode();                                              //Service mode screen is printed to display.
  }
  else if(flowFaultDisplay == true) {
    resolveFlowFault();                                             //Water flow fault display mode is printed to display. It contains fault code instruction and possibility to reset fault code.
  }
  
  //Continuesly read out sensor values, calculate values and alert user if any fault code is set.
  moistureValue1 = moistureSensor1.moistureRead(moistureSensorPort1);                                   //Read moistureSensor1 value to check soil humidity.
  moistureValue2 = moistureSensor2.moistureRead(moistureSensorPort2);                                   //Read moistureSensor2 value to check soil humidity.
  moistureValue3 = moistureSensor3.moistureRead(moistureSensorPort3);                                   //Read moistureSensor3 value to check soil humidity.
  moistureValue4 = moistureSensor4.moistureRead(moistureSensorPort4);                                   //Read moistureSensor4 value to check soil humidity.
  moistureMeanValue = calculateMoistureMean(moistureValue1, moistureValue2, moistureValue3, moistureValue4);    //Mean value from all sensor readouts.
  
  tempValue = humiditySensor.readTemperature(false);                                                    //Read temperature value from DHT-sensor. "false" gives the value in °C.
  //humidValue = humiditySensor.readHumidity();                                                           //Read humidity value from DHT-sensor.
  tempThresholdCompare();
  
  lightRead();                                                                                          //Read light sensor UV value.
  waterLevelRead();                                                                                     //Check water level in water tank.
  alarmMessageDisplay();                                                                                //Print alarm messages to display for any faults that is currently active. Warning messages on display will alert user to take action to solve a certain fault.  

  //Greenhouse program start.
  if(greenhouseProgramStart == true) {                  //When set to 'true' automatic water and lighting control of greenhouse is turned on.
    
    //Check readout light value according to a time cycle and turn led lighting ON/OFF based on the readout.
    unsigned long checkLightNeedCurrent;

    checkLightNeedCurrent = millis();                 //Get current time stamp from millis().
    if(checkLightNeedCurrent - checkLightNeedStart >= checkLightNeedPeriod) {  //Check if time period has elapsed.
      checkLightNeed();                               //Time period has elapsed. Enable/Disable start of LED lighting.
      if(ledLightEnabled == true) {
        ledLightStart();                              //Start(ON) LED lighting.
      }
      else {
        ledLightStop();                               //Stop(OFF) LED lighting.
      }                
      checkLightNeedStart = millis();                 //Get current time stamp from millis() to make it loop.   
    }
    
    //Check readout light value after led lighting has been turned on. This will check if led lighting is working. If not it will set an alarm.
    unsigned long checkLightFaultCurrent;
    
    if(ledLightState == true) {
      checkLightFaultCurrent = millis();              //Get current time stamp from millis().
      if(checkLightFaultCurrent - checkLightFaultStart >= checkLightFaultPeriod) { //Check if time period has elapsed.
        ledLightCheck();                              //Time period has elapsed. Check if LED lighting is working.
        checkLightFaultStart = millis();              //Get current time stamp from millis() to make it loop.                                
      }
    }

    //Check readout moisture value according to a time cycle and turn water pump ON for a certain amount of time based on readout.
    unsigned long checkMoistureCurrent;

    checkMoistureCurrent = millis();                  //Get current time stamp from millis().
    if(checkMoistureCurrent - checkMoistureStart >= checkMoisturePeriod) {    //Check if time period has elapsed.                                                             
      checkWaterNeed();                               //Time period has elapsed. Enable/Disable start of water pump.
      checkMoistureStart = millis();                  //Get current time stamp from millis() to make it loop.                                                                                       
    }

    //Start water pump and let it run for a certain amount of time.
    unsigned long waterPumpTimeCurrent;

    if(waterPumpEnabled == true) {
      waterPumpStart();                               //Start water pump (ON).
      waterPumpTimeStart = millis();                  //Get current time stamp from millis().
      if(waterPumpTimeCurrent - waterPumpTimeStart >= waterPumpTimePeriod) {  //Check if time period has elapsed.
        waterPumpStop();                              //Time period has elapsed. Stop water pump (OFF).
        waterPumpEnabled = false;                     //Disable water pump from running until next time moisture value readout.
        waterPumpTimeCurrent = millis();              //Get current time stamp from millis() to make it loop.
      }
    }

    //Check if water is being pumped when water pump is running by checking the water flow sensor. If not it will set an alarm.
    unsigned long checkWaterFlowCurrent;

    if(pumpState == true) {
      checkWaterFlowStart = millis();                 //Get current time stamp of millis().
      if(checkWaterFlowCurrent - checkWaterFlowStart >= checkWaterFlowPeriod) {     //Check if time period has elapsed.
        waterFlowCheck();                             //Time period has elapsed. Check water flow.
        checkWaterFlowCurrent = millis();             //Get current time stamp from millis() to make it loop.
      }
    }
  }                                                                      
}

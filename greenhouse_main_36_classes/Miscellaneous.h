#ifndef Miscellaneous_H_
#define Miscellaneous_H_
#include "Arduino.h"
#include "Moisture.h"
#include "Lighting.h"
#include "Temperature.h"
#include "Watering.h"
#include "SeeedOLED.h"
#include "Wire.h"

/*
****************************************************************
* Pin setup for hardware connected to Arduino UNO base shield. *
****************************************************************/
//Arduino UNO board I/O pin setup.
static const unsigned char SENSOR_PORT1 = A0;
static const unsigned char SENSOR_PORT2 = A1;
static const unsigned char SENSOR_PORT3 = A2;
static const unsigned char SENSOR_PORT4 = A3;
static const unsigned char DHTPIN = 4;
static const unsigned char rotaryEncoderOutpA = 11;
static const unsigned char rotaryEncoderOutpB = 10;
static const unsigned char pumpRelay = 8;
static const unsigned char flowSensor = 3;
static const unsigned char waterLevelSwitch = 12;
static const unsigned char lightRelay = 6;
static const unsigned char clockSetButton = 7;
static const unsigned char clockModeButton = 2;

//Arduino UNO base shield layout
/*
################### ARDUINO UNO ############################
#|                                                        |#
#|__________________DIGITAL_(PWM_~)_______________________|#
#||X|X|X|GND|13|12|~11|~10|~9| 8|<>| 7|~6|~5| 4|~3| 2|X|X||#
#|                                                        |#
#|                                                        |#
#|     |A3|      |D4|      |D3|      |D2|      |UART|     |#
#|                                                        |#
#|     |A2|      |D8|      |D7|      |D6|      |D5|       |#
#|                                                        |#
#|     |A1|      |I2C|     |I2C|     |I2C|     |I2C|      |#
#|                                                        |#
#|                                                        |#
#|______________POWER_____________________ANALOG IN_______|#
#||A0| | |X|X|X|3.3V|5V|GND|GND|Vin|<>|A0|A1|A2|A3|A4|A5| |#
#|                                                        |#
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


//Moisture.
static const unsigned short MOISTURE_THRESHOLD_LOW = 300;
static const unsigned short MOISTURE_THRESHOLD_HIGH = 700;
static bool moistureDry = false;                              //Fault code is active 'true' if soil moisture is too dry. Internal fault code handled by the greenhouse program.
static bool moistureWet = false;                              //Fault code is active 'true' if soil moisture is too wet. Internal fault code handled by the greenhouse program.

//Temperature readout, humidity readout and temperature threshold adjustment.
//static const uint8_t DHTTYPE = DHT11;                         //Set Arduino model that is being used. DHT11 equals the Arduino UNO.
static unsigned short tempThresholdValue = 60;                //Initial value / 2 for temperature threshold adjusted by rotary encoder. Value 60 / 2 is 30°C.
static unsigned short aLastState;                             //Variable to keep track of rotary direction when adjusting temperature threshold by rotary encoder.
static bool tempValueFault = false;                           //Fault code is active 'true' if read out temperature value is higher than temperature treshold set by rotary encoder. 
static unsigned short TEMP_VALUE_MIN = 28;                    //Temperature value can be set within the boundaries of 14 - 40°C. Temp value is doubled to reduce rotary knob sensitivity.
static unsigned short TEMP_VALUE_MAX = 80;

//Light readout and turn ON/OFF the LED lighting.
static bool ledLightEnabled = false;                          //Enable/Disable LED to start.
static bool ledLightState = false;                            //Show current status of LED lighting. Variable is 'true' when LED lighting is turned ON.
static bool ledLightFault = false;                            //Fault code is active 'true' if LED lights is not emitting light even though they have been turned ON.
static const unsigned short UV_THRESHOLD_VALUE = 4;           //UV threshold value for turning to enable/disable LED lighting to be turned ON/OFF.

//Water pump, flow sensor and water level sensor.
static volatile int rotations;
static bool waterPumpEnabled = false;                         //Enable/Disable water pump to run.
static bool waterPumpState = false;                           //Indicate current status of water pump. Variable is 'true' when water pump is running (ON).
static bool waterLevelFault = false;                          //Fault code is active 'true' if water surface in water tank is too low to let the float sensor float.
static bool waterFlowFault = false;                           //Fault code is active 'true' if water flow is below threshold value when water pump is running (ON). 
static const unsigned short FLOW_THRESHOLD_VALUE = 99;        //Specifies water flow threshold in Liter/hour which must be exceeded to avoid water flow fault code from being activated.



//Internal clock to keep track of current time.
static int currentClockTime = 0;

//Timer interrupts for Timer1 and Timer2.
static unsigned short divider50 = 0;                          //Variable to devide the frequency in which time interrupt fucntion is called by 50.
static unsigned short divider10 = 0;                          //Variable to devide the frequency in which time interrupt fucntion is called by 10.


typedef enum {STARTUP_IMAGE, SET_TIME, READOUT_VALUES, SERVICE_MODE, FLOW_FAULT} displayMode;   //Enum with 'typedef' for the different display modes that can be printed to the OLED display. The 'typedef' makes it possible to pass the display mode name instead of passing an integer that corresponds to that specific display that will be printed to display. 



#endif  /* Miscellaneous_H_ */

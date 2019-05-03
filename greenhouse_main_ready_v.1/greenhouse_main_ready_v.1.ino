/*
*************************
  Included header files
*************************/
#include "Wire.h"
#include "SeeedGrayOLED.h"
#include "multi_channel_relay.h"
#include "DHT.h"
#include "SI114X.h"
#include "MoistureSensor.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"    //Fill in cridentials (password and username) for connecting to local wifi where greenhouse is placed.

/*
****************************************************************
  Pin setup for hardware connected to Arduino UNO base shield.
****************************************************************/
//Pin setup Arduino UNO board.
#define moistureSensorPort1 A0
#define moistureSensorPort2 A1
#define moistureSensorPort3 A2
#define moistureSensorPort4 A3
#define DHTPIN 4
#define rotaryEncoderOutpA 11
#define rotaryEncoderOutpB 10
#define waterFlowSensor 3
#define fanSpeedSensor 13
#define waterLevelSensor 12
#define resetButton 7
#define modeButton 2

//Arduino UNO base shield I/O layout.
/*
  ################### ARDUINO UNO ############################
  #|                                                        |#
  #|__________________DIGITAL_(PWM_~)_______________________|#
  #||X|X|X|GND|13|12|~11|~10|~9| 8|<>| 7|~6|~5| 4|~3| 2|X|X||#
  #|                                                        |#
  #|     |A3|                                               |#
  #|               |D4|      |D3|      |D2|      |UART|     |#
  #|     |A2|                                               |#
  #|               |D8|      |D7|      |D6|      |D5|       |#
  #|     |A1|                                               |#
  #|               |I2C|     |I2C|     |I2C|     |I2C|      |#
  #|     |A0|                                               |#
  #|                                                        |#
  #|______________POWER_____________________ANALOG IN_______|#
  #||A0| | |X|X|X|3.3V|5V|GND|GND|Vin|<>|A0|A1|A2|A3|A4|A5| |#
  #|                                                        |#
  ################### ARDUINO UNO ############################
  ______________________________________________________________________________________________________________________________________________________________
  GROVE connectors                      | DIGITAL (PWM~)                                                                                                        |
  ****************                      | **************                                                                                                        |
  A3:   Moisture Sensor4                | GND:  10 kohm resistor in in series with with 12 (I/O)                                                                |
  A2:   Moisture Sensor3                | 13:   Fan signal cable in parallell with 10 kohm resistor to +5V                                                      |
  A1:   Moisture Sensor2                | 12:   10 kohm resistor parallell with signal wire1 to water tank level switch. Resistor is in series with GND (I/O)   |
  A0:   Moisture Sensor1                | 11~:  Signal wire1 to temperature rotary encoder                                                                      |
  D4:   Humidity & Temperature Sensor   | 10~:  Signal wire2 to temperature rotary encoder                                                                      |
  D8:   'EMPTY'                         |                                                                                                                       |
  I2C:  4-Channel Relay                 | All other (unspecified) of its I/O:s are 'EMPTY'.                                                                     |
  D3:   Water Flow Sensor               |                                                                                                                       |
  D7:   SET-Button                      |                                                                                                                       |                                                                                                                      |
  I2C:  Sunlight Sensor                 | POWER                                                                                                                 |
  D2:   MODE-Button                     | *****                                                                                                                 |
  D6:   'EMPTY'                         | 5V:   Supply wire to water tank level switch in parallell supply wire to temperature rotary encoder.                  |
  I2C:  OLED Display (128x128 px)       | GND:  Ground wire to temperature rotary encoder.                                                                      |
  UART: 'EMPTY'                         |                                                                                                                       |
  D5:   'EMPTY'                         | All other (unspecified) of its I/O:s are 'EMPTY'.                                                                     |
  I2C:  'EMPTY'                         |                                                                                                                       |
                                        |                                                                                                                       |
                                        | ANALOG IN                                                                                                             |
                                        | *********                                                                                                             |
                                        | All I/O:s are 'EMPTY'.                                                                                                |
  ______________________________________|_______________________________________________________________________________________________________________________|

  /*
*********************
  Global variables.
*********************/
/*
  /////////////////////////////////////////////////////////////////////////////
  PARAMETERS ALLOWED TO BE CHANGED TO ALTER THE WAY GREENHOUSE PROGRAM RUNS. //
                                                                           //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  */
//IMPORTANT! Connect to sync internal clock otherwise program will not work properly.
//Fill in username and password in the separate file: arduino_secrets.h

//SOIL MOISTURE.
const unsigned short MOISTURE_THRESHOLD_LOW = 660;                  //Set moisture interval values. When measured moisture value (how much water soil contains) is within this interval soil moisture is considered to be OK for plants.
const unsigned short MOISTURE_THRESHOLD_HIGH = 700;                 //Same as above but upper threshold for what is considered to be OK soil moisture.

//FAN SPEED CONTROL.
const unsigned short HUMIDITY_THRESHOLD_VALUE = 60;                 //Set air humidity threshold value (humidity in procentage, value < 100) for when fan should run at low speed. If measured air humidity is lower than specified value fan will run at low speed mode.

//ALARM TRIGGER VALUES.
//Temperature.
const unsigned short TEMP_THRESHOLD_VALUE = 28;                     //Set temperature threshold value (°C). If measured temperature is above this specified value a temperature alarm is activated. Value 30 means equal to 30°C.
//Water flow.
const unsigned short FLOW_THRESHOLD_VALUE = 250;                    //Variable value specifies the minimum water flow (Liter/hour) required to avoid activating water flow fault.
const unsigned short CHECK_WATER_FLOW_PERIOD = 1500;                //Set for how long time (in milliseconds) after water pump has been activated (turned ON) before program checks the water flow. IMPORTANT: Value must be above 1000, since it takes 1 sec before water flow value is calculated.
//LED lighting.
const unsigned short UV_THRESHOLD_VALUE = 4;                        //Set at which UV-value LED lighting alarm is activated. If UV-value is lower than specified value when LED lighting is ON, an alarm is activated.
const unsigned short CHECK_LIGHT_FAULT_PERIOD = 3000;               //Set delay time (in milliseconds) after LED lighting has been turned ON, before checking if it works. Program checks if measured light value is above a certain level.

//ALLOWED CLOCK TIME TO RUN.
//Specify clock time when fan, LED lighting and water pump is allowd to run. Clock time converted to an intiger (700 = 07:00 and 2335 = 23:35).
unsigned short LIGHT_FAN_START_TIME = 700;                          //Start clock time (after specified time) fan and LED lighting is allowed to be activated (ON).
unsigned short LIGHT_FAN_STOP_TIME = 2300;                          //Stop clock time (after specified time) for when fan and LED lighting is NOT allowed to be activated and is turned OFF if is currently running.
unsigned short PUMP_START_TIME = 800;                               //Start clock time (after specified time) water pump is allowed to be activated (ON).
unsigned short pumpStopTime = 1500;                                 //Stop clock time (after specified time) water pump is NOT allowed to run and is turned OFF.

//LOOP TIME.
//Loop time for how often certain readouts and/or motors  be activated.
const unsigned int CHECK_MOISTURE_PERIOD = 600000;                   //Loop time (in milliseconds) how often soil moisture is being checked and hence water pump is activated (only when soil is too dry).
const unsigned short WATER_PUMP_TIME_PERIOD = 6000;                 //Set time (in milliseconds) how long water pump will run each time it is activated. Fan speed mode is also checked in same interval as water pump.
const unsigned int CHECK_LIGHT_NEED_PERIOD = 5000;                  //Loop time (in milliseconds) how often ligtht and fan need is being checked. Light need is only checking if current time is in allowed interval meanwhile fan also checks if humidity level is too high.
/*
  .................................................................///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Oliver Staberg                                                   //
  ALTEN Sweden AB in cooperation with Västsvenska Handelskammaren  //
  Gothenburg, April 2019.                                          //
  ///////////////////////////////////////////////////////////////////
  /*

  /*
  \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
  OTHER GLOBAL VARIABLES BELOW, DO NOT TOUCH! \\
  //////////////////////////////////////////////
*/

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
//const unsigned int MOISTURE_THRESHOLD_LOW = 650;
//const unsigned int MOISTURE_THRESHOLD_HIGH = 700;

//Temperature and humidity sensor.
const uint8_t DHTTYPE = DHT11;            //DHT11 = Arduino UNO model is being used.
DHT humiditySensor(DHTPIN, DHTTYPE);      //Create humidity sensor from DHT class.
float tempValue;
float humidityValue;                      //Air humidity value.
bool tempValueFault = false;              //Indicate if read out temperature is higher than temperature treshold that has been set by adjusting temperature rotary encoder. Variable is 'false' when read out temperature is below set temperature threshold.
const unsigned short TEMP_VALUE_MIN = 12;                    //Temperature value can be set within the boundaries of 12 - 40°C. Temp value is doubled to reduce rotary knob sensitivity. Values are doubled to increase rotary encoder precision.
const unsigned short TEMP_VALUE_MAX = 40;

//4-Channel Relay
Multi_Channel_Relay relay;                //Relay object created from Multi_Channel_Relay class.
uint8_t WATER_PUMP = 4;                   //Relay channel number where water pump is connected.
uint8_t LED_LIGHTING = 3;                 //Relay channel number where led lighting is connected.
uint8_t FAN = 2;                          //Relay channel number where fan is connected.
uint8_t FAN_LOW_SPEED = 1;                //Relay channel number where fan (low speed control) is connected.

//Rotary encoder to adjust temperature threshold.
unsigned short tempThresholdValue = TEMP_THRESHOLD_VALUE;              //Starting value for temperature threshold adjustment is value specified in TEMP_THRESHOLD_VALUE variable.
int aLastState;

//Debouncing button press, MODE-button (triggers external interrupt when pressed).
volatile unsigned long pressTimePrev;     //Variable to store previous millis() value.
unsigned short DEBOUNCE_TIME_INTERRUPT = 170;             //Delay time before interrupt function is started.

//Debouncing button press, SET-button (normal push button).
unsigned short DEBOUNCE_TIME_BUTTON = 150;

//Light sensor.
SI114X lightSensor = SI114X();            //Light sensor object created.
uint16_t lightValue;                      //Light readout, unit in lumens.
uint16_t uvValue;                         //UV-light readout, UN-scale.
//uint16_t irValue;                       //IR read out not in use.

//LED lighting.
bool ledLightState = false;               //Indicate current status of LED lighting. Variable is 'true' when LED lighting is turned on.
bool ledLightFault = false;               //Indicate if LED lighting is not turned on/not working when LED lighting has been turned on.

//Water pump and flow sensor.
volatile int flowSensorRotations;
unsigned short waterFlowValue = 0;
bool waterPumpState = false;              //Indicate current status of water pump. Variable is 'true' when water pump is running.
bool waterFlowFault = false;              //Indicate if water is being pumped when water pump is running. Variable is 'false' when water flow is above threshold value.
//int FLOW_THRESHOLD_VALUE = 80;              //Variable value specifies the minimum water flow threshold required to avoid setting water flow fault.

//Water level switch.
bool waterLevelFault = false;             //If variable is 'false' water level is OK. If 'true' tank water level is too low.

//Internal clock to keep track of current time.
int currentClockTime = 0;
int hourPointer1 = 0;
int hourPointer2 = 0;
int minutePointer1 = 0;                 //1-digit of minute pointer.
int minutePointer2 = 0;                 //10-digit of minute pointer.
int secondPointer1 = 0;                 //1-digit of second pointer.
int secondPointer2 = 0;                 //10-digit of second pointer.
bool hour2InputMode = true;
bool hour1InputMode = false;
bool minute2InputMode = false;
bool minute1InputMode = false;
bool pushButton = false;

bool clockStartMode = false;
bool clockSetFinished = false;
unsigned short divider10 = 0;
unsigned short divider5 = 0;
bool flashClockPointer = false;         //Variable to create lash clock pointer when in "set clock" mode.

//Alarm messages to display.
bool alarmMessageEnabled = false;       //Enable any alarm to be printed to display. If variable is 'true' alarm is enable to be printed to display.
unsigned long alarmTimePrev = 0;        //Used to read relative time
unsigned long alarmTimePeriod = 2100;   //Variable value specifies in milliseconds, for how long time each warning message will be shown on display before cleared and/or replaced by next warning message.

//Toggle display modes.
bool startupImageDisplay = true;        //Any variable is set to 'true' when that screen mode is currently printed to display.
bool setTimeDisplay = false;
bool readoutValuesDisplay = false;
bool serviceModeDisplay = false;
bool flowFaultDisplay = false;

static bool toggle2 = false;
unsigned short clockTime1 = 0;
unsigned short clockTime2 = 0;
/*
  ---------------------
  |Greenhouse program.|
  --------------------*/
bool greenhouseProgramStart = false;        //If variable is set to 'true', automatic water and lighting control of greenhouse is turned on.
static bool allowRestart = false;
unsigned short actionRegister;

//Water pump.
bool waterPumpEnabled = false;              //Enable/Disable water pump to be activated to pump water.
//unsigned int CHECK_MOISTURE_PERIOD = 20000;        //Loop time, in milliseconds, for how often water pump is activated based upon measured soil moisture value.
unsigned long checkMoistureStart = 0;
//unsigned int CHECK_WATER_FLOW_PERIOD = 1500;
unsigned long checkWaterFlowStart = 0;
//unsigned int WATER_PUMP_TIME_PERIOD = 6000;  //Sets the time for how long water pump will run each time it is activated.
unsigned long waterPumpTimeStart = 0;
bool waterPumpTimeAllowed = false;          //Is set 'true' when current time is inside time interval where water pump is allowed to be turned ON.

//LED lighting.
bool ledLightEnabled = false;               //Enable/Disable start of LED lighting.
//int UV_THRESHOLD_VALUE = 9;                   //UV threshold value for turning LED lighting on/off.
//const unsigned int CHECK_LIGHT_NEED_PERIOD = 5000;   //Loop time for how often measured light value is checked. This enables/disables start of LED lighting.
unsigned long checkLightNeedStart = 0;
//unsigned int CHECK_LIGHT_FAULT_PERIOD = 3000;  //Delay time after LED lighting has been turned ON, before checking if it works.
unsigned long checkLightFaultStart = 0;
bool ledLightTimeAllowed = false;           //Is set 'true' when current time is inside time interval where LED lighting is allowed to be turned ON.

//Fan.
bool fanEnabled = false;                    //Enable/Disable fan to run.
bool fanState = false;
//const unsigned short HUMIDITY_THRESHOLD_VALUE = 60;
bool lowFanSpeedEnabled = false;
unsigned short fanSpeedValue = 0;               //Fan speed readout.
bool fanTimeAllowed = false;                //Is set 'true' when current time is inside time interval where fan is allowed to be turned ON.
bool checkFanSpeed = false;                 //Variable is set 'true' when one second has passed. This makes it possible to calculate fan rpm value.
volatile int fanRotations = 0;
unsigned long timeNow;
unsigned long timePrev = 0;
unsigned long timeDiff;

//Set time for when fan, LED lights and water pump is allowd to run.
//unsigned short LIGHT_FAN_START_TIME = 700;   //Time set as an intiger (700 = 07:00 and 2335 = 23:35).
//unsigned short LIGHT_FAN_STOP_TIME = 2300;
//unsigned short PUMP_START_TIME = 800;
//unsigned short pumpStopTime = 1500;

//Wifi variables to sync internal clock with NTP-server.
int status = WL_IDLE_STATUS;
static bool WiFiConnected = true;
static bool timerInterruptHasSetup = false;
static unsigned int counterRashid = 0;
static unsigned int counterWifiDiscounected = 0;
bool wifiClockCompleted = false;

//Enter your sensitive data in the Secret tab/arduino_secrets.h.
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(194, 58, 203, 20); // gbg1.ntp.se NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

/*
  ============================================================
  || Bitmap image to be printed on OLED display at startup. ||
  ============================================================ */
const unsigned char greenhouse[] PROGMEM = {
  /*
    //Startup image.
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x07, 0xC0,
    0x00, 0x03, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x04, 0xA0,
    0x00, 0x06, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x04, 0xA0,
    0x00, 0x0C, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x40, 0x00, 0x03, 0x00,
    0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0xFC, 0x7C, 0x00, 0x04, 0x00,
    0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80,
    0x00, 0x04, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0xF0, 0x38, 0x00, 0x07, 0x80,
    0x00, 0x06, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x21, 0x00, 0x44, 0x00, 0x00, 0x80,
    0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x44, 0x00, 0x00, 0x80,
    0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x38, 0x00, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x07, 0x81, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x0A, 0x81, 0x00, 0x08, 0x00, 0x04, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x54, 0x00, 0x04, 0x80,
    0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x01, 0x01, 0xF0, 0x24, 0x00, 0x1F, 0x80,
    0x00, 0x0C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80,
    0x00, 0x0C, 0x31, 0x80, 0x00, 0x00, 0x00, 0x12, 0x00, 0x7F, 0x81, 0xF0, 0x3C, 0x00, 0x04, 0x00,
    0x00, 0x0C, 0x61, 0x80, 0x00, 0x00, 0x00, 0x0C, 0x08, 0x00, 0x00, 0x10, 0x40, 0x00, 0x04, 0x00,
    0x00, 0x0C, 0x61, 0x80, 0x00, 0x00, 0x00, 0x04, 0x0F, 0x07, 0x00, 0x10, 0x40, 0x00, 0x07, 0x80,
    0x00, 0x07, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x1A, 0x06, 0x08, 0x81, 0xE0, 0x7C, 0x00, 0x03, 0x00,
    0x00, 0x03, 0x81, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x09, 0x08, 0x80, 0x00, 0x00, 0x00, 0x05, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x08, 0x83, 0x00, 0x34, 0x00, 0x05, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x09, 0x0F, 0xE4, 0x80, 0x54, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x06, 0x00, 0x04, 0x80, 0x54, 0x00, 0x07, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x0A, 0x09, 0x07, 0xF0, 0x38, 0x00, 0x04, 0x00,
    0x00, 0x0F, 0xFF, 0x8F, 0xF8, 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x80,
    0x00, 0x0F, 0xFF, 0x80, 0x30, 0x00, 0x00, 0x0A, 0x05, 0x04, 0x80, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x00, 0xC0, 0x00, 0x00, 0x1A, 0x07, 0x00, 0x00, 0x01, 0x24, 0x00, 0x04, 0x80,
    0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x1A, 0x08, 0x2F, 0x81, 0x00, 0xC4, 0x00, 0x0F, 0x00,
    0x00, 0x00, 0x01, 0x86, 0x00, 0x00, 0x00, 0x0C, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x01, 0x8F, 0xF8, 0x00, 0x00, 0x0C, 0x0F, 0x0E, 0x01, 0xF0, 0x00, 0x00, 0x04, 0x80,
    0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x4A, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x04, 0x80,
    0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x52, 0x05, 0x20, 0x80, 0xD0, 0x70, 0x00, 0x03, 0x00,
    0x00, 0x0F, 0xFF, 0x08, 0x88, 0x00, 0x00, 0x22, 0x0D, 0x20, 0x81, 0x50, 0x0C, 0x00, 0x03, 0x80,
    0x00, 0x0F, 0xFC, 0x08, 0x88, 0x00, 0x00, 0x00, 0x06, 0x20, 0x81, 0x50, 0x32, 0x00, 0x0A, 0x80,
    0x00, 0x00, 0x00, 0x08, 0x88, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0xE0, 0x40, 0x00, 0x08, 0x80,
    0x00, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x12, 0x26, 0x00, 0x00, 0x00, 0x44, 0x00, 0x07, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x29, 0x00, 0x01, 0x10, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x03, 0xF0, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0xFE, 0x08, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0,
    0x00, 0x1F, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x10, 0x00, 0x11, 0x00, 0xF1, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x0F, 0xFF, 0x0F, 0xF8, 0x00, 0x00, 0x7E, 0x00, 0x20, 0x81, 0x50, 0x00, 0x00, 0x0F, 0x00,
    0x00, 0x0F, 0xDF, 0x88, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x20, 0x81, 0x51, 0xFC, 0x00, 0x09, 0x80,
    0x00, 0x07, 0xEF, 0x88, 0x00, 0x00, 0x00, 0x1F, 0x04, 0x20, 0x80, 0x30, 0x44, 0x00, 0x0E, 0x80,
    0x00, 0x07, 0xF7, 0x88, 0x08, 0x00, 0x00, 0x12, 0x8D, 0x3F, 0x80, 0x00, 0x44, 0x10, 0x00, 0x00,
    0x00, 0x03, 0xFB, 0x80, 0x08, 0x00, 0x00, 0x12, 0x8D, 0x00, 0x07, 0x80, 0x38, 0x1E, 0x0F, 0x80,
    0x00, 0x03, 0xFD, 0x80, 0x08, 0x00, 0x00, 0x0C, 0x06, 0x24, 0x80, 0x70, 0x00, 0x00, 0x08, 0x00,
    0x00, 0x01, 0xFE, 0xC0, 0x08, 0x00, 0x00, 0x00, 0x08, 0x24, 0x80, 0xF1, 0x7C, 0x0C, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x1F, 0xF8, 0x00, 0x00, 0x5E, 0x06, 0x24, 0x87, 0x00, 0x00, 0x12, 0x0F, 0x00,
    0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F, 0x83, 0xC0, 0x3C, 0x12, 0x08, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x02, 0x0C, 0x00, 0x00, 0x70, 0x40, 0x0C, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x02, 0x04, 0x00, 0x81, 0xE0, 0x40, 0x04, 0x00, 0x00,
    0x00, 0x01, 0xFF, 0x83, 0xC0, 0x02, 0xC0, 0x7E, 0x0D, 0x00, 0x86, 0x00, 0x3C, 0x1A, 0x0E, 0x80,
    0x00, 0x07, 0xFF, 0x8C, 0x40, 0x03, 0x40, 0x00, 0x0D, 0x00, 0x80, 0x00, 0x40, 0x0A, 0x09, 0x80,
    0x00, 0x06, 0x00, 0x0E, 0x40, 0x01, 0x80, 0x00, 0x06, 0x3F, 0x80, 0x00, 0x40, 0x00, 0x08, 0x80,
    0x00, 0x0C, 0x00, 0x01, 0xE0, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x0E, 0x00, 0x00,
    0x00, 0x0C, 0x00, 0x00, 0x38, 0x02, 0xC0, 0x00, 0x01, 0x1F, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0x31, 0x01, 0x00, 0x7C, 0x1E, 0x1F, 0x80,
    0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0xF8, 0x3F, 0x20, 0x81, 0xF0, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x03, 0x00, 0x03, 0xC1, 0x04, 0x00, 0x20, 0x80, 0x00, 0x04, 0x0A, 0x0F, 0x80,
    0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x02, 0x02, 0x00, 0x11, 0x00, 0xE0, 0x78, 0x1A, 0x04, 0x00,
    0x01, 0xFF, 0xFF, 0x83, 0xA0, 0x03, 0xC2, 0x02, 0x08, 0x0E, 0x01, 0x10, 0x00, 0x1A, 0x07, 0x80,
    0x01, 0xFF, 0xFF, 0x81, 0x80, 0x00, 0x42, 0x02, 0x0F, 0x00, 0x01, 0x11, 0xFC, 0x0C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x40, 0x03, 0xC1, 0x8C, 0x00, 0x00, 0x00, 0xE0, 0x20, 0x14, 0x07, 0x80,
    0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x78, 0x05, 0x00, 0x00, 0x00, 0x20, 0x1A, 0x04, 0x80,
    0x00, 0x00, 0x00, 0x0F, 0xC0, 0x02, 0x40, 0x00, 0x0D, 0x00, 0x01, 0x60, 0x20, 0x0A, 0x07, 0xE0,
    0x00, 0x01, 0xFF, 0x80, 0x00, 0x07, 0x83, 0xFE, 0x06, 0x00, 0x01, 0x51, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0x07, 0xFF, 0x80, 0x00, 0x01, 0xC0, 0x0C, 0x00, 0x04, 0x00, 0x90, 0x00, 0x00, 0x01, 0x80,
    0x00, 0x06, 0x00, 0x0F, 0xC0, 0x03, 0x40, 0x18, 0x09, 0x07, 0x80, 0x00, 0x00, 0x08, 0x0F, 0x00,
    0x00, 0x0C, 0x00, 0x02, 0x40, 0x02, 0xC0, 0x60, 0x1E, 0x03, 0x00, 0xF0, 0x00, 0x1A, 0x0F, 0x00,
    0x00, 0x0C, 0x00, 0x02, 0x40, 0x01, 0x80, 0xC0, 0x07, 0x04, 0x81, 0x00, 0x00, 0x1A, 0x01, 0x80,
    0x00, 0x0C, 0x00, 0x01, 0xC0, 0x02, 0xC3, 0x80, 0x0D, 0x04, 0x81, 0x00, 0x00, 0x0C, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x01, 0x80, 0x02, 0xC3, 0xFE, 0x0D, 0x03, 0x01, 0xF0, 0x7E, 0x10, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x02, 0xC0, 0x01, 0x80, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x45, 0x1E, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x02, 0xC0, 0x0A, 0x00, 0x00, 0x30, 0x04, 0x81, 0x50, 0x45, 0x00, 0x00, 0x00,
    0x00, 0x0F, 0xFF, 0x81, 0x80, 0x07, 0xC3, 0xFC, 0x07, 0x06, 0x81, 0x50, 0x38, 0x1E, 0x00, 0x00,
    0x00, 0x0F, 0xFF, 0x81, 0xC0, 0x00, 0x00, 0x06, 0x0E, 0x00, 0x00, 0x50, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x38, 0x03, 0x80, 0xE0, 0x3C, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x07, 0x04, 0x00, 0x60, 0x40, 0x1C, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0x03, 0xC0, 0x00, 0x00, 0x04, 0x0E, 0x04, 0x04, 0x90, 0x40, 0x12, 0x00, 0x00,
    0x00, 0x03, 0xE1, 0x03, 0xE0, 0x00, 0x03, 0xF8, 0x30, 0x07, 0x84, 0x90, 0x40, 0x3E, 0x00, 0x00,
    0x00, 0x06, 0x21, 0x82, 0x50, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x10, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x21, 0x82, 0x50, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x00, 0x00, 0x1E, 0x00, 0x00,
    0x00, 0x0C, 0x21, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x01, 0x7C, 0x1A, 0x00, 0x00,
    0x00, 0x08, 0x21, 0x87, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x16, 0x00, 0x00,
    0x00, 0x0C, 0x21, 0x80, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x03, 0x00, 0x60, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x21, 0x02, 0xC0, 0x00, 0x00, 0x42, 0x0F, 0x12, 0x80, 0x70, 0xFC, 0x10, 0x00, 0x00,
    0x00, 0x06, 0x27, 0x03, 0x40, 0x00, 0x00, 0x42, 0x10, 0x14, 0x81, 0xC0, 0x40, 0x1E, 0x00, 0x00,
    0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x42, 0x10, 0x08, 0x80, 0x60, 0x00, 0x08, 0x00, 0x00,
    0x00, 0x00, 0xF8, 0x03, 0xC0, 0x00, 0x00, 0x3C, 0x1F, 0x00, 0x00, 0x70, 0x3C, 0x1A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x40, 0x1A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x02, 0x00, 0xE0, 0x40, 0x0C, 0x00, 0x00,
    0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x15, 0x06, 0x81, 0x13, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xE1, 0x0F, 0xC0, 0x00, 0x00, 0x40, 0x13, 0x06, 0x81, 0x10, 0x00, 0x0C, 0x00, 0x00,
    0x00, 0x06, 0x21, 0x82, 0x40, 0x00, 0x00, 0x40, 0x00, 0x03, 0x01, 0x10, 0x7E, 0x12, 0x00, 0x00,
    0x00, 0x0C, 0x21, 0x83, 0xC0, 0x00, 0x00, 0x20, 0x48, 0x04, 0x00, 0xE0, 0x45, 0x12, 0x00, 0x00,
    0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x7E, 0x48, 0x07, 0x80, 0x00, 0x45, 0x1F, 0x80, 0x00,
    0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x07, 0xF0, 0x45, 0x0E, 0x00, 0x00,
    0x00, 0x04, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x38, 0x10, 0x00, 0x00,
    0x00, 0x06, 0x23, 0x00, 0x00, 0x00, 0x02, 0x7E, 0x00, 0x00, 0x84, 0x80, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x81, 0x7C, 0x0E, 0x00, 0x00,
    0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x10, 0x07, 0x07, 0xF0, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1F, 0x04, 0x80, 0x00, 0x04, 0x1E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0F, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x5F, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00,
    0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x01, 0x01, 0x01, 0x01, 0xFC, 0x1A, 0x00, 0x00,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x06, 0x81, 0xF0, 0x00, 0x1A, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x02, 0x80, 0x00, 0x00, 0x4C, 0x00, 0x00,
    0x00, 0x0F, 0xFF, 0x80, 0x00, 0x00, 0x03, 0xFE, 0x74, 0x00, 0x00, 0xD0, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x0F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x64, 0x0E, 0x17, 0x81, 0x50, 0x70, 0x7E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x01, 0x03, 0x01, 0x50, 0x88, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x04, 0x80, 0xE1, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x04, 0x81, 0x11, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xF1, 0x04, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x01, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x1C, 0x00, 0xF1, 0x24, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x51, 0x24, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0x81, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x01, 0x31, 0x24, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x1C, 0x04, 0x01, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0x01, 0x80, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x0F, 0x81, 0xC0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x40, 0x01, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00, 0x70, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x60, 0x03, 0x00, 0x00, 0x00, 0x01, 0xD0, 0x00, 0x00, 0x07, 0x80, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x30, 0x07, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x07, 0x80, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x1C, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x71, 0xFC, 0x00, 0x00, 0x00,
    0x00, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  */
};

/*
  ======================================================
  || Initialize OLED display and show startup images. ||
  ====================================================== */
void viewStartupImage() {
  Serial.println("startupImageDisplay");
  SeeedGrayOled.clearDisplay();                         //Clear display.

  //Make everything is shut down.
  waterPumpStop();                                                //Stop(OFF) water pump.
  ledLightStop();                                                 //Stop(OFF) LED lighting.
  fanStop();                                                      //Stop(OFF) fan.

  /*
      //Startup image.
      SeeedGrayOled.drawBitmap(greenhouse, (128*128)/8);   //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
      delay(4000);                                    //Image shown for 4 seconds.
      SeeedGrayOled.clearDisplay();                       //Clear the display.
  */

  startupImageDisplay = false;                            //Clear current screen display state.
  setTimeDisplay = true;                                  //Set next display mode to be printed to display.

  stringToDisplay(0, 0, "GREENHOUSE v.1");

  if (WiFiConnected == true) {    //Connected to wifi, print following to display.
    stringToDisplay(2, 0, "is connected");
    stringToDisplay(4, 0, "to Wifi.");
    stringToDisplay(6, 0, "Internal clock");
    stringToDisplay(7, 0, "is synced with");
    stringToDisplay(8, 0, "NTP-server.");

    //Set variables.
    hour2InputMode = false;                               //Set state in next display mode.
    minute1InputMode = false;                             //Minute pointer1 has been set. Time set is done.
    clockStartMode = true;                                //Start clock. Clock starts ticking.
    clockSetFinished = true;
  }
  else {                          //Not connected to wifi, print following to display.
    stringToDisplay(2, 0, "is Not connected");
    stringToDisplay(4, 0, "to Wifi!");
    stringToDisplay(6, 0, "Internal clock");
    stringToDisplay(7, 0, "must be set by");
    stringToDisplay(8, 0, "user input.");

    //Set variables.
    hour2InputMode = true;                                //Set state in next display mode.
  }

  stringToDisplay(10, 0, "Program is");
  stringToDisplay(11, 0, "booting up..");
  stringToDisplay(14, 0, "           Alten");
  stringToDisplay(15, 0, "     april, 2019");
  delay(9000);
  SeeedGrayOled.clearDisplay();
}

/*
  =======================================
  || Print number variable to display. ||
  ======================================= */
void numberToDisplay(unsigned char x, unsigned char y, unsigned short variable) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  SeeedGrayOled.setTextXY(x, y);                  //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(variable);              //Print value to display.
}

/*
  ===================================
  || Print custom text to display. ||
  =================================== */
void stringToDisplay(unsigned char x, unsigned char y, char* text) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  SeeedGrayOled.setTextXY(x, y);                  //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString(text);                  //Print text to display.
}

/*
  ======================================================
  || Clear any character/s (print blanks) at display. ||
  ====================================================== */
void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  for (int i = 0; i < numOfBlanks; i++) {         //Print blank space to display. Each loop one blank space is printed.
    SeeedGrayOled.setTextXY(x, y);                //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
    SeeedGrayOled.putString(" ");                 //Blank symbol.
    y += 8;                                       //Increase column cordinate to print next blank space in the same row.
  }
}

/*
  ========================================================================
  || VALUE READOUT DISPLAY MODE. Print read out values to OLED display. ||
  ======================================================================== */
void viewReadoutValues() {
  //Clear symbols from previous display mode.
  blankToDisplay(0, 0, 2);
  blankToDisplay(2, 9, 7);
  blankToDisplay(3, 5, 11);
  blankToDisplay(4, 6, 8);
  blankToDisplay(5, 9, 5);
  blankToDisplay(6, 9, 4);
  blankToDisplay(7, 5, 9);
  blankToDisplay(8, 9, 5);
  blankToDisplay(9, 5, 5);
  blankToDisplay(10, 8, 5);
  blankToDisplay(11, 0, 16);

  blankToDisplay(13, 0, 16);

  blankToDisplay(14, 7, 9);

  stringToDisplay(0, 2, "READOUT VALUES");          //Print current display state to upper right corner of display.

  //Printing read out values from the greenhouse to display.
  /*************************************
    |Moisture mean value and soil status.|
  *************************************/
  stringToDisplay(2, 0, "Moisture:");
  numberToDisplay(2, 10, moistureMeanValue);    //Moisture mean value calculated from all four moisture sensor readouts.

  stringToDisplay(3, 0, "Soil:");               //Prints "Dry", "OK" or "Wet" to display based on soil humidity.
  if (moistureDry == true) {
    stringToDisplay(3, 10, "Dry   ");
  }
  else if (moistureDry == false && moistureWet == false) {
    stringToDisplay(3, 10, "OK    ");
  }
  else if (moistureWet == true) {
    stringToDisplay(3, 10, "Wet   ");
  }

  /***************************
    |Light and UV-light values.|
  ***************************/
  SeeedGrayOled.setTextXY(4, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Light:");            //Print string to display.
  SeeedGrayOled.setTextXY(4, 10 * 8);
  SeeedGrayOled.putNumber(lightValue);          //Print light value in the unit, lux, to display.
  stringToDisplay(4, 14, "lm");

  SeeedGrayOled.setTextXY(5, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("UV-light:");         //Print string to display.
  SeeedGrayOled.setTextXY(5, 10 * 8);
  SeeedGrayOled.putNumber(uvValue);             //Print light value in the unit, lux, to display.
  stringToDisplay(5, 14, "UN");

  /********************
    |Air humidity value.|
  ********************/
  stringToDisplay(6, 0, "Humidity:");
  numberToDisplay(6, 10, humidityValue);      //Air humidity value, unit in %.
  stringToDisplay(6, 13, "pct");

  /*************************************************************************
    |Temperature value and temperature threshold value set by rotary encoder.|
  *************************************************************************/
  stringToDisplay(7, 0, "Temp:");
  numberToDisplay(7, 10, tempValue);            //Temperature value.
  stringToDisplay(7, 14, "*C");

  stringToDisplay(8, 0, "Temp lim:");
  SeeedGrayOled.setTextXY(8, 10 * 8);
  SeeedGrayOled.putNumber(tempThresholdValue);  //Print temperature threshold value to display. Temp value is doubled to reduce rotary sensitivity and increase knob rotation precision. Value 24 corresponds to 12°C.
  stringToDisplay(8, 14, "*C");

  /*************************
    |Water flow sensor value.|
  *************************/
  SeeedGrayOled.setTextXY(9, 0);                    //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Flow:");              //Print string to display.
  SeeedGrayOled.setTextXY(9, 6 * 8);
  SeeedGrayOled.putNumber(waterFlowValue);          //Print water flow value to display.
  stringToDisplay(9, 10, "ml/min");

  /*****************
    |Fan speed value.|
  *****************/
  SeeedGrayOled.setTextXY(10, 0);                     //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Fan spd:");                //Print string to display.
  SeeedGrayOled.setTextXY(10, 9 * 8);
  SeeedGrayOled.putNumber(fanSpeedValue);                //Print water flow value to display.
  stringToDisplay(10, 13, "rpm");


  /****************
    |Current action.|
  ****************/
  switch (actionRegister) {
    case 1:
      stringToDisplay(12, 0, "Check light need");
      break;
    case 2:
      stringToDisplay(12, 0, "Check water need");
      break;
    case 4:
      stringToDisplay(12, 0, "Pumping water.. ");
      break;
    case 8:
      blankToDisplay(12, 0, 16);
      break;
  }

  /*****************************
    |Space for any active alarms.|
  *****************************/
  stringToDisplay(14, 0, "Alarms:");
}

/*
  ==========================================
  || Read light values from light sensor. ||
  ========================================== */
void lightRead() {
  unsigned short value = 0;
  lightValue = lightSensor.ReadVisible();
  value = lightSensor.ReadUV();

  //Only update uvValue if not equal to zero to avoid an uvValue of zero because it is not updated as frequently as the other light sensor.
  if (value != 0 && ledLightState == true) {
    uvValue = value;
  }
  //irValue = lightSensor.ReadIR();
}

/*
  ===========================================================================================
  || Check current clock time to enable/disable start of LED lighting, fan and water pump. ||
  =========================================================================================== */
void checkTimePermission() {
  //LED lighting and fan are allowed to run in this time window.
  if (currentClockTime >= LIGHT_FAN_START_TIME && currentClockTime < LIGHT_FAN_STOP_TIME) {
    ledLightTimeAllowed = true;       //LED lighting is allowed to be turned on.
    fanTimeAllowed = true;            //Fan is allowed to run.
    Serial.println("LED lighting allowed.");
    Serial.println("Fan allowed.");
  }
  else {
    ledLightTimeAllowed = false;       //LED lighting is not allowed to be turned on.
    fanTimeAllowed = false;            //Fan is not allowed to run.
  }

  //Water pump allowed to run in below time window.
  if (currentClockTime >= PUMP_START_TIME && currentClockTime < pumpStopTime) {
    if (moistureDry == true) {
      waterPumpTimeAllowed = true;    //Water pump is allowed to run.
    }
  }
  else {
    waterPumpTimeAllowed = false;   //Water pump is not allowed to run.
  }
}

/*
  ===========================
  || Turn ON LED lighting. ||
  =========================== */
void ledLightStart() {
  relay.turn_on_channel(LED_LIGHTING);                                 //Turn on LED lighting.
  ledLightState = true;                                           //Update current LED lighting state, 'true' means lighting is on.
  Serial.println("LED lighting ON");
}

/*
  =======================================
  || Check if LED lighting is working. ||
  ======================================= */
//Alarm if light read out value does not get above light threshold when LED lighting is turned on.
void ledLightCheck() {
  if (ledLightState == true && uvValue < UV_THRESHOLD_VALUE) {
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
  relay.turn_off_channel(LED_LIGHTING);                                //Turn off LED lighting.
  ledLightState = false;                                        //Update current LED lighting state, 'false' means lighting is off.
  Serial.println("LED lighting OFF");
}


/*
  ======================================================================================
  || Check current clock time and light need to enable/disable start of LED lighting. ||
  ====================================================================================== */
void checkLightNeed() {
  //Check if LED lighting is allowed to run now.
  checkTimePermission();            //Check if LED lighting is allowed to be turned ON (inside allowed time interval). Fan run permission is also checked here be since it has the same defined time interval as LED lighting.

  if (ledLightTimeAllowed == true && fanTimeAllowed == true) {
    ledLightEnabled = true;         //Enable LED lighting to be turned on.
    fanEnabled = true;              //Enable fan to run.
  }
  Serial.println("Check light need.");
}

/*
  ==============================
  || Read water level switch. ||
  ============================== */
void waterLevelRead() {
  waterLevelFault = digitalRead(waterLevelSensor);                  //If variable is 'false' water level is OK. If 'true' tank water level is too low.
}

/*
  ================================================================================================================
  || Count number of rotations flow sensor propeller does. Function runs every time interrupt pin is triggered. ||
  ================================================================================================================ */
void waterFlowCount() {
  //Interrupt function to count number of rotations that flow sensor makes when water is being pumped.
  flowSensorRotations++;
}

/*
  ======================================================
  || Calculate water flow when water pump is running. ||
  ====================================================== */
void waterFlow() {
  waterFlowValue = (float(flowSensorRotations) * 60 * 1000) / 3467;   //(water flow value in ml/min) = ((total rotations during 1 sec * 60 sec) / (number of rotations it takes to pump 1 liter of water) * (1000 to convert value to milli liter).
  flowSensorRotations = 0;

  Serial.print("flowSensorRotations: ");
  Serial.println(flowSensorRotations);

  Serial.print("timer: ");
  Serial.println(millis());


  Serial.print("waterFlowValue: ");
  Serial.println(waterFlowValue);
}

/*
  ===============================================
  || Start water pump, read water flow sensor. ||
  =============================================== */
void waterPumpStart() {
  relay.turn_on_channel(WATER_PUMP);          //Start water pump.
  waterPumpState = true;                  //Update current water pump state, 'true' means water pump is running.
  Serial.println("Water pump ON");
}

/*
  ======================
  || Stop water pump. ||
  ====================== */
void waterPumpStop() {
  relay.turn_off_channel(WATER_PUMP);         //Stop water pump.
  waterPumpState = false;               //Update current water pump state, 'false' means water pump not running.
  waterFlowValue = 0;                   //Clear water flow value when pump is not running to prevent any old value from water flow sensor to be printed to display.
  Serial.println("Water pump OFF");
}

/*
  =========================================================================
  || Check if water flow is above a certain amount when pump is running. ||
  ========================================================================= */
void waterFlowCheck() {
  Serial.println("Check water flow");
  if (waterFlowValue < FLOW_THRESHOLD_VALUE) { //Check current water flow.
    waterFlowFault = true;              //Set fault code.
    Serial.println("Water flow Fault");
  }
  else {
    waterFlowFault = false;             //Clear fault code.
    Serial.println("Water flow OK");
  }
}

/*
  ======================================
  || Enable/Disable water pump start. ||
  ====================================== */
void checkWaterNeed() {
  checkTimePermission();              //Check if water pump is allowed to be running when needed (inside allowed time interval).
  if (waterPumpTimeAllowed == true) {

    //Water pump is enabled if soil moisture is too dry or at the same time as no water related fault codes are set.
    if (moistureDry == true && moistureWet == false) {
      if (waterLevelFault == false && waterFlowFault == false) {  //Make sure no water related fault codes are set.
        waterPumpEnabled = true;      //Enable water pump to run to pump water if needed.
      }
    }
  }
  Serial.println("Check water need.");
}

/*
  =====================================================================================================
  || Count number of rotations fan blades does. Function runs every time interrupt pin is triggered. ||
  ===================================================================================================== */
void fanRotationCount() {
  //Interrupt function to count number of rotations that flow sensor makes when water is being pumped.
  fanRotations++;
}

/*
  ==========================
  || Calculate fan speed. ||
  ========================== */
void fanRpm() {
  //Calculate fan rpm (rotations/minute) by counting number of rotations that fan blades make. Sensor is connected to interrupt pin.
  //Function called once every second only when fan is running.
  if (wifiClockCompleted == true) {
    fanSpeedValue = fanRotations * 60 / 4;           //Calculate number of rotations fan blade have made during the time that passed since last measurement.
  }
  else {
    fanSpeedValue = fanRotations * 60 / 2;           //Calculate number of rotations fan blade have made during the time that passed since last measurement.
  }
  fanRotations = 0;
}

/*
  =========================================================================
  || Check and compare air-humidity to decide which speed to run fan at. ||
  ========================================================================= */
void humiditySpeedControl() {
  if (humidityValue < HUMIDITY_THRESHOLD_VALUE) {
    lowFanSpeedEnabled = true;                                  //Activate low fan speed mode if air humidity is below humidity threshold value.
  }
  else {
    lowFanSpeedEnabled = false;
  }
}



/*
  ==================
  || Turn ON fan. ||
  ================== */
void fanStart() {
  if (lowFanSpeedEnabled == true) {
    relay.turn_off_channel(FAN);                                //Make sure other fan mode is deactivated.
    relay.turn_on_channel(FAN_LOW_SPEED);                       //Turn ON fan, low speed mode.

    fanState = true;                                            //Update current fan state, 'true' means lighting is on.
    Serial.println("Fan (low speed) is ON");
  }
  else if (lowFanSpeedEnabled == false) {
    relay.turn_off_channel(FAN_LOW_SPEED);                      //Make sure other fan mode is deactivated.
    relay.turn_on_channel(FAN);                                 //Turn ON fan, normal speed mode.
    fanState = true;                                            //Update current fan state, 'true' means lighting is on.
    Serial.println("Fan is ON");
  }
}

/*
  ===================
  || Turn OFF fan. ||
  =================== */
void fanStop() {
  relay.turn_off_channel(FAN);                                  //Turn OFF fan no matter what fan speed mode that is currently running.
  relay.turn_off_channel(FAN_LOW_SPEED);
  fanState = false;                                             //Update current fan state to indicate it is turned OFF.
  Serial.println("Fan is OFF");
}

/*
  ===========================================================================================================================================================
  || Timer interrupt triggered with frequency of 10 Hz used as second ticker for internal clock and to flash clock pointer values when in "set time" mode. ||
  =========================================================================================================================================================== */
ISR(RTC_CNT_vect) {
  RTC.INTFLAGS = 0x3;  //Clearing OVF and CMP interrupt flags.

  //if (greenhouseProgramStart == true) {
  divider10++;

  //Timer interrupt triggered with a frequency of 10 Hz.
  if (divider10 >= 10) {                  //This part of the function will run once every second and therefore will provide a 1 Hz pulse to feed the second pointer.
    divider10 = 0;                       //Clear divider variable.

    //Internal clock.
    secondPointer1++;                     //Increase second pointer every time this function runs.

    //Second pointer.
    if (secondPointer1 == 10) {           //If 1-digit second pointer reaches a value of 10 (elapsed time is 10 seconds).
      secondPointer2++;                   //Increase 10-digit second pointer.
      secondPointer1 = 0;                 //Clear 1-digit pointer.
    }
    if (secondPointer2 == 6) {            //If 10-digit pointer reaches a value of 6 (elapsed time is 60 seconds).
      minutePointer1++;                   //Increase minute pointer.
      secondPointer2 = 0;                 //Clear 10-digit second pointer.
    }
    //Minute pointer.
    if (minutePointer1 == 10) {           //If 1-digit minute pointer reaches a value of 10 (elapsed time is 10 minutes).
      minutePointer2++;                   //Increase 10-digit minute pointer.
      minutePointer1 = 0;                 //Clear 1-digit minute pointer.
    }
    if (minutePointer2 == 6) {            //If 10-digit minute pointer reaches a value of 6 (elapsed time is 60 minutes).
      hourPointer1++;                     //Increase 1-digit hour pointer.
      minutePointer2 = 0;                 //Clear 10-digit minute pointer.
    }
    //Hour pointer.
    if (hourPointer1 == 10) {             //If 1-digit hour pointer reaches a value of 10 (elapsed time is 10 hours).
      hourPointer2++;                     //Increase 10-digit hour pointer.
      hourPointer1 = 0;                   //Clear 1-digit hour pointer.
    }
    if (hourPointer2 == 2 && hourPointer1 == 4) { //If 1-digit and 10-digit hourPointer combined reaches 24 (elapsed time is 24 hours).
      hourPointer1 = 0;                           //Clear both hour digits.
      hourPointer2 = 0;
    }

    //Convert clock pointer into single int variable. Value of this variable represent clock time.
    currentClockTime = 0;
    currentClockTime += (hourPointer2 * 1000);
    currentClockTime += (hourPointer1 * 100);
    currentClockTime += (minutePointer2 * 10);
    currentClockTime += minutePointer1;

    if (currentClockTime < 60) {          //Prevent clock time from seeing 00:00 as less than 23:00.
      currentClockTime += 2400;
    }
    wifiClockCompleted = false;

    //Functions for calculation fan speed and water flow is triggered every second. The delay time of one second is used as time base for the calculation.
    //Calculating fan speed on.
    if (fanState == true) {
      fanRpm();
    }

    //Time delay for calculating water flow.
    if (waterPumpState == true) {
      waterFlow();
    }
  }
  //}
}

/*
  ===============================================================
  || Set current time by using SET- and MODE-buttons as input. ||
  =============================================================== */
void setClockTime() {
  //Set current clock time by toggling each hour pointer and minute pointer individualy.
  if (pushButton == true) {
    delay(DEBOUNCE_TIME_BUTTON);                                                 //Delay to avoid contact bounce.
    resetStartupVariables();
  }
}

/*
  ======================
  || Reset clock time ||
  ====================== */
void resetClockTime() {
  //Stop clock and reset all clock pointers.
  clockStartMode = false;                       //Stop clock from ticking.
  currentClockTime = 0;
  hourPointer1 = 0;
  hourPointer2 = 0;
  minutePointer1 = 0;
  minutePointer2 = 0;
  secondPointer1 = 0;
  secondPointer2 = 0;
}

/*
  ======================================================================================
  || Toggle set modes and screen display modes when modeButton is being pressed. ||
  ====================================================================================== */
void toggleDisplayMode() {
  //Debouncing button press to avoid multiple interrupts, display toggles.
  if ((millis() - pressTimePrev) >= DEBOUNCE_TIME_INTERRUPT) {

    //Toggle display modes every time MODE-button is pressed.
    if (setTimeDisplay == true) {
      Serial.println("setTimeDisplay");
      if (hour2InputMode == true) {
        hour2InputMode = false;                 //Hour pointer2 has been set.
        hour1InputMode = true;                  //Continue by setting hour pointer1.
        Serial.println("hour2InputMode");
      }
      else if (hour1InputMode == true) {
        hour1InputMode = false;                 //Hour pointer1 has been set.
        minute2InputMode = true;                //Continue by setting minute pointer2.
        Serial.println("hour1InputMode");
      }
      else if (minute2InputMode == true) {
        minute2InputMode = false;               //Minute pointer2 has been set.
        minute1InputMode = true;                //Continue by setting minute pointer1.
        Serial.println("minute2InputMode");
      }
      else if (minute1InputMode == true) {
        minute1InputMode = false;               //Minute pointer1 has been set. Time set is done.
        clockStartMode = true;                  //Start clock. Clock starts ticking.
        clockSetFinished = true;
        Serial.println("minute1InputMode");
      }
      else if (clockSetFinished == true) {
        clockSetFinished = false;               //Clear current state in display mode.
        setTimeDisplay = false;                 //Clear current display mode.
        readoutValuesDisplay = true;            //Set next display mode to be printed to display.
        alarmMessageEnabled = true;             //Enable any alarm message to be printed to display.
        greenhouseProgramStart = true;          //Start greenhouse program.
        Serial.println("clockSetFinished");
      }
    }
    else if (readoutValuesDisplay == true) {
      readoutValuesDisplay = false;               //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      alarmMessageEnabled = false;                //Disable any alarm message from being printed to display.
      //SeeedGrayOled.clearDisplay();                   //Clear display.
      serviceModeDisplay = true;                  //Set next display mode to be printed to display.
      Serial.println("readoutValuesDisplay");
    }
    else if (serviceModeDisplay == true) {
      serviceModeDisplay = false;                 //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedGrayOled.clearDisplay();                   //Clear display.
      readoutValuesDisplay = true;                //Set next display mode to be printed to display.
      alarmMessageEnabled = true;                 //Enable any alarm message from being printed to display.
      Serial.println("serviceModeDisplay");
    }
    else if (flowFaultDisplay == true) {
      //flowFaultDisplay = false;                   //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedGrayOled.clearDisplay();                   //Clear display.
      Serial.println("flowFaultDisplay");
      if (allowRestart == true) {
        flowFaultDisplay = false;                   //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
        waterFlowFault = false;                   //Clear water flow fault code.
        allowRestart = false;
        Serial.println("Go to setTimeDisplay");
      }
    }

    //Check if water flow fault code is active. If active enter flow fault display to handle fault code.

    if (waterFlowFault == true) {
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
  blankToDisplay(0, 0, 7);
  stringToDisplay(0, 7, "SET CLOCK");     //Print current display state to upper right corner of display.

  if (clockStartMode == false) {
    stringToDisplay(2, 0, "Use controls to ");
    stringToDisplay(3, 0, "set curr. time: ");
    blankToDisplay(4, 0, 16);
    stringToDisplay(5, 0, "ENCODER = +/-   ");
    blankToDisplay(6, 0, 16);
    stringToDisplay(7, 0, "MODE = confirm  ");
    blankToDisplay(8, 0, 16);
    stringToDisplay(9, 0, "RESET = clear   ");
    blankToDisplay(10, 0, 16);


    //Pointer separator character.
    stringToDisplay(11, 22, ":");
    stringToDisplay(11, 25, ":");
    blankToDisplay(12, 0, 16);
    blankToDisplay(13, 0, 16);
    blankToDisplay(14, 0, 16);
    blankToDisplay(15, 0, 16);
  }
  else if (clockStartMode == true) {
    //Print further instructions when clock start has been activated.
    stringToDisplay(2, 0, "Clock is ticking");
    blankToDisplay(3, 0, 16);
    stringToDisplay(4, 0, "Auto watering,  ");
    stringToDisplay(5, 0, "lighting & hum- ");
    stringToDisplay(6, 0, "idity control   ");
    stringToDisplay(7, 0, "is ready to run ");
    blankToDisplay(8, 0, 16);
    stringToDisplay(9, 0, "Time is:        ");
    blankToDisplay(10, 0, 16);
    blankToDisplay(12, 0, 16);
    stringToDisplay(13, 0, "Press MODE to  ");
    stringToDisplay(14, 0, "continue.      ");
    blankToDisplay(15, 0, 16);

    //Pointer separater character flash.
    if (flashClockPointer == true) {
      SeeedGrayOled.setTextXY(11, 22 * 8);
      SeeedGrayOled.putString(" ");

      SeeedGrayOled.setTextXY(11, 25 * 8);
      SeeedGrayOled.putString(" ");
    }
    else {
      SeeedGrayOled.setTextXY(11, 22 * 8);
      SeeedGrayOled.putString(":");

      SeeedGrayOled.setTextXY(11, 25 * 8);
      SeeedGrayOled.putString(":");
    }
  }

  blankToDisplay(11, 0, 4);
  blankToDisplay(11, 12, 4);
  numberToDisplay(11, 20, hourPointer2);
  numberToDisplay(11, 21, hourPointer1);
  numberToDisplay(11, 23, minutePointer2);
  numberToDisplay(11, 24, minutePointer1);
  numberToDisplay(11, 26, secondPointer2);
  numberToDisplay(11, 27, secondPointer1);

  //Second pointers.
  /*SeeedGrayOled.setTextXY(11, 26 * 8);
    SeeedGrayOled.putNumber(secondPointer2);                   //Print second digit of second pointer value to display.
    SeeedGrayOled.setTextXY(11, 27 * 8);
    SeeedGrayOled.putNumber(secondPointer1);                   //Print first digit of second pointer value to display.
  */
  //Print and flash individual clock time pointers to display which clock parameter that is currently set.
  //Hour pointer2.
  if (hour2InputMode == true) {
    if (flashClockPointer == true) {
      SeeedGrayOled.setTextXY(12, 20 * 8);
      SeeedGrayOled.putString(" ");                             //Clear display where 10-digit hour pointer value is located.
    }
    else {
      SeeedGrayOled.setTextXY(12, 20 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
      SeeedGrayOled.putString("_");                             //Clear display where 10-digit hour pointer value   }
    }
  }

  //pushButton = digitalRead(resetButton);                        //Check if SET-button is being pressed.

  //Hour pointer1.
  if (hour1InputMode == true) {
    if (flashClockPointer == true) {
      blankToDisplay(12, 0, 16);
      //SeeedGrayOled.setTextXY(12, 21 * 8);
      //SeeedGrayOled.putString(" ");                             //Clear display where 1-digit hour pointer value is located.
    }
    else {
      SeeedGrayOled.setTextXY(12, 21 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
      SeeedGrayOled.putString("_");                   //Print 1-digit hour pointer value to display.
    }
  }

  //Minute pointer2.
  if (minute2InputMode == true) {
    if (flashClockPointer == true) {
      blankToDisplay(12, 0, 16);
      //SeeedGrayOled.setTextXY(12, 23 * 8);
      //SeeedGrayOled.putString(" ");
    }
    else {
      SeeedGrayOled.setTextXY(12, 23 * 8);
      SeeedGrayOled.putString("_");
    }
  }

  //Minute pointer1.
  if (minute1InputMode == true) {
    if (flashClockPointer == true) {
      blankToDisplay(12, 0, 16);
      //SeeedGrayOled.setTextXY(12, 24 * 8);
      //SeeedGrayOled.putString(" ");
    }
    else {
      SeeedGrayOled.setTextXY(12, 24 * 8);
      SeeedGrayOled.putString("_");
    }
  }
}

/*
  ============================================================================================================
  || Compare read out temperature with temperature threshold that has been set by adjusting rotary encoder. ||
  ============================================================================================================ */
void tempThresholdCompare() {
  if (tempValue > tempThresholdValue || tempValue < TEMP_VALUE_MIN) {                             //Compare read out temperature value with temperature threshold value set by rotary encoder.
    tempValueFault = true;                                         //If measured temperature is higher than temperature threshold that has been set, variable is set to 'true' to alert user.
  }
  else {
    tempValueFault = false;
  }
}

/*
  ========================================================================================================
  || Read temperature threshold set by rotary encoder respectively increas/decrease clock cursor value. ||
  ======================================================================================================== */
void rotaryEncoderRead() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  volatile int virtualPosition = 0;

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(rotaryEncoderOutpB) == LOW)
    {
      virtualPosition-- ; // Could be -5 or -10
    }
    else {
      virtualPosition++ ; // Could be +5 or +10
    }

    //Adjust cursor value when in set clock time display mode.
    if (setTimeDisplay == true) {
      if (hour2InputMode == true) {
        hourPointer2 += virtualPosition;                            //Increase/Decrease cursor value whenever rotary encoder knob is turned.
        if (hourPointer2 == 3) {                                    //If 10-digit hour pointer reaches 3, clear digit.
          hourPointer2 = 0;
        }
        else if (hourPointer2 < 0) {                                //No negative cursor value allowed.
          hourPointer2 = 0;
        }
      }
      else if (hour1InputMode == true) {
        hourPointer1 += virtualPosition;                            //Increase/Decrease cursor value whenever rotary encoder knob is turned.

        if (hourPointer2 == 2) {                                    //If hour pointer2 is equal to 2, hour pointer 1 is only allowed to reach a maximum value of 4.
          if (hourPointer1 == 5) {
            hourPointer1 = 0;
          }
        }

        if (hourPointer1 == 10 || hourPointer1 < 0) {               //If 1-digit hour pointer reaches 10 or is less than zero, clear digit.
          hourPointer1 = 0;
        }
      }
      else if (minute2InputMode == true) {
        minutePointer2 += virtualPosition;                          //Increase/Decrease cursor value whenever rotary encoder knob is turned.
        if (minutePointer2 == 6 || minutePointer2 < 0) {            //If 10-digit minute pointer reaches 6 or is less than zero, clear 10-digit minute pointer.
          minutePointer2 = 0;
        }
      }
      else if (minute1InputMode == true) {
        minutePointer1 += virtualPosition;                          //Increase/Decrease cursor value whenever rotary encoder knob is turned.
        if (minutePointer1 == 10 || minutePointer1 < 0) {           //If 10-digit minute pointer reaches a value of 10, clear 1-digit minute pointer.
          minutePointer1 = 0;
        }
      }

      //Replace clock time represenation. When current clock time is 24 hours is replaced with 00.
      if (clockStartMode == true) {
        if (hourPointer2 == 2 && hourPointer1 == 4) {               //If 10-digit hour pointer reaches a value of 2 and 1-digit hour pointer reaches a value of 4 (elapsed time is 24 hours).
          hourPointer2 = 0;                                         //Clear both hour pointer values.
          hourPointer1 = 0;
        }
      }
    }

    //Adjust temperature threshold when in readout display mode.
    else if (readoutValuesDisplay == true) {
      tempThresholdValue += virtualPosition;

      if (tempThresholdValue >= TEMP_VALUE_MAX) {
        tempThresholdValue = TEMP_VALUE_MAX;
      }
      else if (tempThresholdValue <= TEMP_VALUE_MIN) {
        tempThresholdValue = TEMP_VALUE_MIN;
      }
    }

    // Keep track of when we were here last (no more than every 5ms)
    lastInterruptTime = interruptTime;
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

  if (alarmMessageEnabled == true) {                      //Any alarm can only be printed to display if variable is set to 'true'.
    /******************
      |Water flow fault.|
    ******************/
    if (alarmTimeDiff <= alarmTimePeriod) {
      SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to which row that will be cleared.
      SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      if (waterFlowFault == true) {                       //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(15, 0);
        SeeedGrayOled.putString("NO WATER FLOW");             //Print fault message to display.
      }

      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(15, 0);                        //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");          //Clear row to enable other warnings to be printed to display.
      }
    }

    /**********************
      |Low water tank level.|
    **********************/
    if (alarmTimePeriod < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 2) {
      if (waterLevelFault == true) {                        //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to which row that will be cleared.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedGrayOled.setTextXY(15, 0);
        SeeedGrayOled.putString("LOW WATER LEVEL");             //Print fault message to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      }
    }

    /*******************
      |Temperature fault.|
    *******************/
    if (alarmTimePeriod * 2 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 3) {
      if (tempValueFault == true) {                         //If fault variable is set to 'true', fault message is printed to display.
        Serial.println("ALARMA!!");
        if (tempValue > tempThresholdValue) {
          SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to which row that will be cleared.
          SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
          SeeedGrayOled.setTextXY(15, 0);
          SeeedGrayOled.putString("HIGH TEMPERATURE");            //Print fault message to display.
        }
        else if (tempValue < TEMP_VALUE_MIN) {
          SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to the warning message will be printed.
          SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
          SeeedGrayOled.setTextXY(15, 0);
          SeeedGrayOled.putString("LOW TEMPERATURE");            //Print fault message to display.
        }
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      }
    }

    /********************
      |LED lighting fault.|
    ********************/
    if (alarmTimePeriod * 3 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 4) {
      if (ledLightFault == true) {                          //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to which row that will be cleared.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedGrayOled.setTextXY(15, 0);
        SeeedGrayOled.putString("LED NOT WORKING");             //If measured water flow is below a certain value without the water level sensor indicating the water tank is empty, there is a problem with the water tank hose. "Check water hose!" is printed to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      }
    }

    if (alarmTimeDiff > alarmTimePeriod * 4) {
      SeeedGrayOled.setTextXY(15, 0);                          //Set cordinates to the warning message will be printed.
      SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      alarmTimePrev = millis();                           //Read millis() value to reset time delay calculation.
    }
  }
}

/*
  ===========================================================================
  || SERVICE MODE DISPLAY MODE. Print service mode screen to OLED display. ||
  =========================================================================== */
void viewServiceMode() {
  //Clear symbols from previous display mode.
  blankToDisplay(0, 0, 4);

  blankToDisplay(2, 6, 2);
  blankToDisplay(3, 0, 16);
  blankToDisplay(4, 9, 7);
  blankToDisplay(5, 3, 3);
  blankToDisplay(5, 8, 1);
  blankToDisplay(5, 12, 3);
  blankToDisplay(6, 3, 3);
  blankToDisplay(6, 8, 1);
  blankToDisplay(6, 12, 3);

  blankToDisplay(7, 0, 16);
  blankToDisplay(8, 12, 4);
  blankToDisplay(9, 10, 6);

  blankToDisplay(10, 9, 7);
  blankToDisplay(11, 10, 6);
  blankToDisplay(12, 11, 5);

  blankToDisplay(13, 0, 16);

  stringToDisplay(0, 4, "SERVICE MODE");  //Print current display state to upper right corner of display.

  if (wifiClockCompleted == false) {
    blankToDisplay(15, 0, 16);
  }

  //Display clock.
  stringToDisplay(2, 0, "Clock:");

  //Hour pointerS.
  SeeedGrayOled.setTextXY(2, 8 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  SeeedGrayOled.setTextXY(2, 9 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.

  stringToDisplay(2, 10, ":");

  //Minute pointers.
  SeeedGrayOled.setTextXY(2, 11 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(minutePointer2);                  //Print 10-digit hour pointer value to display.
  SeeedGrayOled.setTextXY(2, 12 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(minutePointer1);                  //Print 1-digit hour pointer value to display.

  stringToDisplay(2, 13, ":");

  //Second pointers.
  SeeedGrayOled.setTextXY(2, 14 * 8);
  SeeedGrayOled.putNumber(secondPointer2);                  //Print second digit of second pointer value to display.
  SeeedGrayOled.setTextXY(2, 15 * 8);
  SeeedGrayOled.putNumber(secondPointer1);                  //Print first digit of second pointer value to display.


  //Display moisture sensor values.
  SeeedGrayOled.setTextXY(4, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Moisture:");                     //Print text to display.

  SeeedGrayOled.setTextXY(5, 0);
  SeeedGrayOled.putString("S1[");                           //Print text to display.
  SeeedGrayOled.setTextXY(5, 3 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue1);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(5, 6 * 8);
  SeeedGrayOled.putString("],");

  SeeedGrayOled.setTextXY(5, 9 * 8);
  SeeedGrayOled.putString("S2[");                           //Print text to display.
  SeeedGrayOled.setTextXY(5, 12 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue2);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(5, 15 * 8);
  SeeedGrayOled.putString("]");

  SeeedGrayOled.setTextXY(6, 0);
  SeeedGrayOled.putString("S3[");                           //Print text to display.
  SeeedGrayOled.setTextXY(6, 3 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue3);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(6, 6 * 8);
  SeeedGrayOled.putString("],");

  SeeedGrayOled.setTextXY(6, 9 * 8);
  SeeedGrayOled.putString("S4[");                           //Print text to display.
  SeeedGrayOled.setTextXY(6, 12 * 8);                         //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue4);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(6, 15 * 8);
  SeeedGrayOled.putString("]");

  //Fault code status.
  stringToDisplay(8, 0, "Fault codes:");
  SeeedGrayOled.setTextXY(9, 0);
  SeeedGrayOled.putString("tempValue:");                    //Print text to display.
  SeeedGrayOled.setTextXY(9, 12 * 8);                       //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(tempValueFault);                  //Print tempValueFault status.

  SeeedGrayOled.setTextXY(10, 0);
  SeeedGrayOled.putString("ledLight:");                     //Print text to display.
  SeeedGrayOled.setTextXY(10, 12 * 8);                      //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(ledLightFault);                   //Print ledLightFault status.

  SeeedGrayOled.setTextXY(11, 0);
  SeeedGrayOled.putString("waterFlow:");                    //Print text to display.
  SeeedGrayOled.setTextXY(11, 12 * 8);                      //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(waterFlowFault);                  //Print waterFlowFault status.

  SeeedGrayOled.setTextXY(12, 0);
  SeeedGrayOled.putString("waterLevel:");                   //Print text to display.
  SeeedGrayOled.setTextXY(12, 12 * 8);                      //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(waterLevelFault);                 //Print waterLevelFault status.

  SeeedGrayOled.setTextXY(14, 0);
  SeeedGrayOled.putString("Wifi conn.: ");

  SeeedGrayOled.setTextXY(14, 12 * 8);
  if (wifiClockCompleted == true) {
    SeeedGrayOled.putString("Yes");
    SeeedGrayOled.setTextXY(15, 0);
    SeeedGrayOled.putString("*Clock in sync ");
  }
  else {
    SeeedGrayOled.putString("NO ");
    SeeedGrayOled.setTextXY(15, 0);
    SeeedGrayOled.putString("*No clock sync!");
  }
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
  for (int i = 0; i < sizeof(moistureValues) / sizeof(int); i++) { //Looping through all measured moisture values to find the highest and lowest moisture values.
    if (moistureValues[i] > moistureMax) {                  //Identify the highest measured moisture value.
      moistureMax = moistureValues[i];
      maxIndex = i;                                         //Identify which moisture sensor that has the max value to be able to delete it from mean moisture value calculation.
    }

    if (moistureValues[i] <= moistureMin) {                 //Identify the lowest measured moisture value.
      moistureMin = moistureValues[i];
      minIndex = i;                                         //Identify which moisture sensor that has the min value to be able to delete it from mean moisture value calculation.
    }
  }

  //Remove maximum and minimum moisture values from moisture array.
  moistureValues[minIndex] = 0;
  moistureValues[maxIndex] = 0;

  for (int i = 0; i < sizeof(moistureValues) / sizeof(int); i++) {
    moistureSum += moistureValues[i];                       //Sum the remaining moisture sensor values.
  }
  moistureMean = moistureSum / 2;                           //Calculate mean moisture value with max and min values excluded.

  //Evaluate soil humidity based on moisture mean value.
  if (moistureMean <= MOISTURE_THRESHOLD_LOW) {               //Soil humidity is too low.
    moistureDry = true;                                     //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureWet = false;
  }
  else if (moistureMean > MOISTURE_THRESHOLD_LOW && moistureMean <= MOISTURE_THRESHOLD_HIGH) { //Soil humidity is good.
    moistureWet = false;                                    //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureDry = false;
  }
  else if (moistureMean > MOISTURE_THRESHOLD_HIGH) {          //Soil humidity is to high.
    moistureWet = true;                                     //Variables used by checkWaterNeed-function to determine if water pump should be enabled.
    moistureDry = false;
  }
  return moistureMean;
}

/*
  ==========================
  || Reset all variables. ||
  ==========================*/
void resetStartupVariables() {
  //Resetting all variables.
  if (setTimeDisplay == true) {     //When in set clock mode perform this type of reset.
    greenhouseProgramStart = false;
    resetClockTime();
    ledLightEnabled = false;
    pushButton = false;

    clockStartMode = false;
    clockSetFinished = false;

    ledLightState = false;
    ledLightFault = false;

    waterPumpState = false;
    waterFlowFault = false;

    hour2InputMode = true;
    hour1InputMode = false;
    minute2InputMode = false;
    minute1InputMode = false;
    pushButton = false;

    clockStartMode = false;
    clockSetFinished = false;
    alarmMessageEnabled = false;

    startupImageDisplay = false;
    setTimeDisplay = true;
    readoutValuesDisplay = false;
    serviceModeDisplay = false;
    flowFaultDisplay = false;

    waterPumpEnabled = false;
    ledLightEnabled = false;
    fanEnabled = false;
    fanState = false;
    actionRegister = 8;
  }
  else if (flowFaultDisplay == true) {               //If getting a water flow fault perform this type of reset without stopping the clock and let the value readout continue.
    minute1InputMode = false;               //Minute pointer1 has been set. Time set is done.
    clockStartMode = true;                  //Start clock. Clock starts ticking.
    clockSetFinished = true;

    ledLightEnabled = false;

    ledLightState = false;
    ledLightFault = false;

    waterPumpState = false;
    waterFlowFault = false;

    pushButton = false;

    alarmMessageEnabled = false;

    startupImageDisplay = false;
    setTimeDisplay = true;
    readoutValuesDisplay = false;
    serviceModeDisplay = false;
    flowFaultDisplay = false;

    waterPumpEnabled = false;
    ledLightEnabled = false;
    fanEnabled = false;
    fanState = false;
    actionRegister = 8;
  }
}

/*
  =========================================================================
  || FLOW FAULT DISPLAY MODE. Print service mode screen to OLED display. ||
  ========================================================================= */
void resolveFlowFault() {
  //Clear symbols from previous display mode.
  blankToDisplay(0, 0, 2);
  blankToDisplay(1, 0, 16);
  blankToDisplay(2, 13, 3);
  blankToDisplay(3, 0, 16);

  blankToDisplay(5, 15, 1);

  blankToDisplay(7, 14, 2);
  blankToDisplay(8, 0, 16);
  blankToDisplay(9, 5, 11);
  blankToDisplay(10, 0, 16);

  blankToDisplay(12, 15, 1);
  blankToDisplay(13, 11, 5);
  blankToDisplay(14, 0, 16);
  blankToDisplay(15, 12, 4);


  stringToDisplay(0, 2, "RSLV FLOWFAULT");          //Print current display state to upper right corner of display.

  stringToDisplay(2, 0, "Chk hardware!");

  stringToDisplay(4, 0, "* Water in hose?");
  stringToDisplay(5, 0, "* Hose tangled?");
  stringToDisplay(6, 0, "* Vacum in tank?");
  stringToDisplay(7, 0, "* Any leakage?");

  stringToDisplay(9, 0, "DONE?");

  stringToDisplay(11, 0, "Press SET-button");
  stringToDisplay(12, 0, "keep it pressed");
  stringToDisplay(13, 0, "to restart.");

  stringToDisplay(15, 0, "Restart: ");

  actionRegister = 8;     //Clear action register printed to display.

  static bool toggle1 = false;
  if (pushButton == true) {
    allowRestart = true;
    stringToDisplay(15, 9, "YES");
    delay(4000);
    resetStartupVariables();
  }
  else {
    stringToDisplay(15, 9, "NO ");
  }
}


/*
  ================================================================
  || WiFi functions for posting readout values to server below. ||
  ================================================================ */
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

bool connectWiFi() {
  bool cnt = false;
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
  unsigned int tryConnectingCounter = 0;
  while ((tryConnectingCounter++ < 6) && (status != WL_CONNECTED)) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 1 second for connection:
    delay(1000);
  }

  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  if (1 == Udp.begin(localPort))cnt = true;
  //delay(10000);
  Serial.print(cnt);
  Serial.println(" connectionStatus");
  return cnt;
}

void setupTimerInterrupt() {
  // put your setup code here, to run once:

  cli();                                                                  //Stop any external interrups.

  //RTC setup:

  while (RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register.
    Serial.println("waiting for 1");
  }
  RTC.CLKSEL = 0x00;        //32.768 kHz signal from OSCULP32K selected.
  RTC.PERL = 0x0A;                         //Lower part of 16,384 value in PER-register (PERL) to be used as overflow value to reset the RTC counter.
  RTC.PERH = 0x10;                         //Upper part of 16,384 value in PER-register (PERH) to be used as overflow value to reset the RTC counter.
  RTC.INTCTRL = (RTC.INTCTRL & 0b11111100) | 0b01;      //Enable interrupt-on-counter overflow by setting OVF-bit in INCTRL register.
  while (RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register.
    Serial.println("waiting for 2");
  }
  RTC.CTRLA = 0x05;           //PRESCALER set to 1024 (0b0) Not using prescaler, CORREN enabled (0b100),  RTCEN bit set to 1 (0b1).

  while (RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register.
    Serial.println("waiting for 3");
  }
  Serial.println("RTC config complete");

  sei();                                                        //Allow external interrupt again.
}

void getTimeOverNetwork() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(500);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    unsigned int currentTime;
    unsigned short currentHour;
    unsigned short currentMinute;
    unsigned short currentSecond;

    currentHour = (epoch % 86400) / 3600 + 2;     //Added two hours to compensate for summer time.
    currentMinute = (epoch % 3600) / 60;
    currentSecond = epoch % 60;

    //Determine if hour value currently has two digits and specify it.
    if ((currentHour / 10) >= 1) {
      hourPointer2 = currentHour / 10;

      //Determine lower hour digit when current hour value consists of two digits.
      hourPointer1 = currentHour - hourPointer2 * 10;
    }
    else {
      hourPointer2 = 0;
      hourPointer1 = currentHour;
    }

    //Determine if minute value currently has two digits and specify it.
    if ((currentMinute / 10) >= 1) {
      minutePointer2 = currentMinute / 10;

      //Determine lower minute digit when current minute value consists of two digits.
      minutePointer1 = currentMinute - minutePointer2 * 10;
    }
    else {
      minutePointer2 = 0;
      minutePointer1 = currentMinute;
    }

    //Determine if second value currently has two digits and specify it.
    if (currentSecond / 10 >= 1) {
      secondPointer2 = currentSecond / 10;

      //Determine lower second digit when current second value consists of two digits.
      secondPointer1 = currentSecond - secondPointer2 * 10;
    }
    else {
      secondPointer2 = 0;
      secondPointer1 = currentSecond;
    }


    if (fanState == true) {
      fanRpm();
    }

    //Time delay for calculating water flow.
    if (waterPumpState == true) {
      waterFlow();
    }

    //Convert clock pointer into single int variable. Value of this variable represent clock time.
    currentClockTime = 0;
    currentClockTime += (hourPointer2 * 1000);
    currentClockTime += (hourPointer1 * 100);
    currentClockTime += (minutePointer2 * 10);
    currentClockTime += minutePointer1;

    if (currentClockTime < 60) {          //Prevent clock time from seeing 00:00 as less than 23:00.
      currentClockTime += 2400;
    }
    wifiClockCompleted = true;
  }
}

void setTime() {
  if (WiFiConnected) {
    //check status
    getTimeOverNetwork();
    if (WiFi.status() != WL_CONNECTED) counterWifiDiscounected++;
    else counterWifiDiscounected = 0;

  } else if (!timerInterruptHasSetup) {

    WiFi.end();
    setupTimerInterrupt();
    timerInterruptHasSetup = true;
  }
  if (counterWifiDiscounected > 10) WiFiConnected = false;
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress & address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

/*
*******************************
  Arduino program setup code.
*******************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //OLED display setup.
  Wire.begin();
  SeeedGrayOled.init(SH1107G);
  SeeedGrayOled.clearDisplay();                         //Clear display.
  SeeedGrayOled.setVerticalMode();
  SeeedGrayOled.setNormalDisplay();                     //Set display to normal mode (non-inverse mode).
  SeeedGrayOled.setTextXY(0, 0);                        //Set cordinates where to print text to display.
  SeeedGrayOled.putString("GREENHOUSE v.1");
  SeeedGrayOled.setTextXY(2, 0);                        //Set cordinates where to print text to display.
  SeeedGrayOled.putString("Attempting to");             //Print text to display.
  SeeedGrayOled.setTextXY(4, 0);
  SeeedGrayOled.putString("connect to Wifi");
  SeeedGrayOled.setTextXY(7, 0);
  SeeedGrayOled.putString("IMPORTANT!");
  SeeedGrayOled.setTextXY(9, 0);
  SeeedGrayOled.putString("Specify Wifi");
  SeeedGrayOled.setTextXY(11, 0);
  SeeedGrayOled.putString("credentials in");
  SeeedGrayOled.setTextXY(13, 0);
  SeeedGrayOled.putString("file: arduino_-");
  SeeedGrayOled.setTextXY(15, 0);
  SeeedGrayOled.putString("secrets.h");
  delay(1000);
  //Wifi setup.
  connectWiFi();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection failed");
    WiFi.end();
    setupTimerInterrupt();
    WiFiConnected = false;
    timerInterruptHasSetup = true;
  }

  //Declaring I/O-ports.
  pinMode(moistureSensorPort1, INPUT);
  pinMode(moistureSensorPort2, INPUT);
  pinMode(moistureSensorPort3, INPUT);
  pinMode(moistureSensorPort4, INPUT);

  pinMode(waterFlowSensor, INPUT);
  pinMode(fanSpeedSensor, INPUT);

  pinMode(waterLevelSensor, INPUT);

  pinMode(rotaryEncoderOutpA, INPUT);
  pinMode(rotaryEncoderOutpB, INPUT);
  aLastState = digitalRead(rotaryEncoderOutpA);      //Read initial position value.

  pinMode(resetButton, INPUT);
  pinMode(modeButton, INPUT);

  //Interupt pins.
  attachInterrupt(13, fanRotationCount, RISING);  //Initialize interrupt to water flow sensor to calculate water flow pumped by water pump.
  attachInterrupt(11, rotaryEncoderRead, LOW); //Initialize interrupt to toggle set modes when in clock set mode or toggling screen display mode when greenhouse program is running. Interrupt is triggered by modeButton being pressed.
  attachInterrupt(3, waterFlowCount, RISING);  //Initialize interrupt to enable calculation of fan speed when it is running.
  attachInterrupt(2, toggleDisplayMode, RISING); //Initialize interrupt to toggle set modes when in clock set mode or toggling screen display mode when greenhouse program is running. Interrupt is triggered by modeButton being pressed.

  humiditySensor.begin();                           //Initializing humidity sensor.

  lightSensor.Begin();                              //Initializing light sensor.

  relay.begin(0x11);

  while (!lightSensor.Begin()) {
    Serial.println("lightSensor is not ready!");
    delay(1000);
  }
  Serial.println("lightsensor is ready!");
}

/*
*******************************************
  Arduino program main code to be looped.
*******************************************/
void loop() {
  // put your main code here, to run repeatedly:

  //Set current time and toggle between different screen display modes.
  pushButton = digitalRead(resetButton);                        //Check if RESET-button is being pressed.

  //Syncronize clock time with NTP-server or initiate and run using internal timer in case wifi is not available.
  setTime();

  //Print current clock time.
  Serial.print(hourPointer2);
  Serial.print(hourPointer1);
  Serial.print(": ");
  Serial.print(minutePointer2);
  Serial.print(minutePointer1);
  Serial.print(": ");
  Serial.print(secondPointer2);
  Serial.println(secondPointer1);
  Serial.print("wifi: ");
  Serial.println(WiFi.SSID());    //FIXA SÅ ATT WIFI-NAMNET STÅR HÄR!!

  //Different functions to run depending of which display mode that is currently active.
  if (startupImageDisplay == true) {
    viewStartupImage();                                             //Initialize the OLED Display and show startup images.
  }
  else if (setTimeDisplay == true) {                                //Display time set screen only if current time has not been set.
    setClockTime();
    setClockDisplay();
  }
  else if (readoutValuesDisplay == true) {                          //Only display read out values after current time on internal clock, has been set.
    viewReadoutValues();                                            //Print read out values from the greenhouse to display.
  }
  else if (serviceModeDisplay == true) {
    viewServiceMode();                                              //Service mode screen is printed to display.
  }
  else if (flowFaultDisplay == true) {
    resolveFlowFault();                                             //Water flow fault display mode is printed to display. It contains fault code instruction and possibility to reset fault code.

    waterPumpStop();                                                //Stop(OFF) water pump.
    ledLightStop();                                                 //Stop(OFF) LED lighting.
    fanStop();                                                      //Stop(OFF) fan.
  }

  //Greenhouse program start. When set to 'true' sensor readouts are enabled and automatic water and lighting control of greenhouse is turned ON.
  if (greenhouseProgramStart == true) {
    //Continuesly read out sensor values, calculate values and alert user if any fault code is set. This part of program is only run when greenhouse program has started, greenhouseProgramStart set 'true'.
    moistureValue1 = moistureSensor1.moistureRead(moistureSensorPort1);                                   //Read moistureSensor1 value to check soil humidity.
    moistureValue2 = moistureSensor2.moistureRead(moistureSensorPort2);                                   //Read moistureSensor2 value to check soil humidity.
    moistureValue3 = moistureSensor3.moistureRead(moistureSensorPort3);                                   //Read moistureSensor3 value to check soil humidity.
    moistureValue4 = moistureSensor4.moistureRead(moistureSensorPort4);                                   //Read moistureSensor4 value to check soil humidity.
    moistureMeanValue = calculateMoistureMean(moistureValue1, moistureValue2, moistureValue3, moistureValue4);    //Mean value from all sensor readouts.

    tempValue = humiditySensor.readTemperature(false);                                                    //Read temperature value from DHT-sensor. "false" gives the value in °C.

    humidityValue = humiditySensor.readHumidity();                                                           //Read humidity value from DHT-sensor.
    tempThresholdCompare();

    //Read light sensor with a less frequency than the rest of the value readouts.
    unsigned long readLightCurrent;

    lightRead();                                                                                          //Read light sensor, light and UV value.

    waterLevelRead();                                                                                     //Check water level in water tank.

    alarmMessageDisplay();                                                                                //Print alarm messages to display for any faults that is currently active. Warning messages on display will alert user to take action to solve a certain fault.

    //Check readout light value according to a time cycle and turn led lighting ON/OFF based on the readout.
    unsigned long checkLightNeedCurrent;

    checkLightNeedCurrent = millis();                 //Get current time stamp from millis().
    if (checkLightNeedCurrent - checkLightNeedStart >= CHECK_LIGHT_NEED_PERIOD) { //Check if time period has elapsed.
      if (actionRegister != 4) {                  //If water pump is not running set follwing value to register.
        actionRegister = 1;                         //Register to print what action that is currently performed in the greenhouse program.
      }
      checkLightNeed();                               //Time period has elapsed. Enable/Disable start of LED lighting.
      if (ledLightEnabled == true) {
        ledLightStart();                              //Start(ON) LED lighting.
      }
      else if (ledLightEnabled == false) {
        ledLightStop();                               //Stop(OFF) LED lighting.
      }
      if (fanEnabled == true) {
        fanStart();                                   //Start(ON) fan.
      }
      else {
        fanStop();                                    //Stop(OFF) LED lighting.
      }
      checkLightNeedStart = millis();                 //Get current time stamp from millis() to make it loop.
    }

    //Check readout light value after led lighting has been turned on. This will check if led lighting is working. If not it will set an alarm.
    unsigned long checkLightFaultCurrent;

    if (ledLightState == true) {
      checkLightFaultCurrent = millis();              //Get current time stamp from millis().
      if (checkLightFaultCurrent - checkLightFaultStart >= CHECK_LIGHT_FAULT_PERIOD) { //Check if time period has elapsed.
        ledLightCheck();                              //Time period has elapsed. Check if LED lighting is working.
        checkLightFaultStart = millis();              //Get current time stamp from millis() to make it loop.
      }
    }

    //Check readout moisture value according to a time cycle and enable water pump to start.
    unsigned long checkMoistureCurrent;

    checkMoistureCurrent = millis();                  //Get current time stamp from millis().
    if (checkMoistureCurrent - checkMoistureStart >= CHECK_MOISTURE_PERIOD) {   //Check if time period has elapsed.
      if (actionRegister != 4) {                  //If water pump is not running set follwing value to register.
        actionRegister = 2;                         //Register to print what action that is currently performed in the greenhouse program.
      }
      checkWaterNeed();                               //Time period has elapsed. Enable/Disable start of water pump.
      humiditySpeedControl();                         //Check air humidity to activate any of the two fan speed modes.
      //Start water pump.
      if (waterPumpEnabled == true) {
        actionRegister = 4;                         //Register to print what action that is currently performed in the greenhouse program.
        waterPumpStart();                               //Start water pump (ON).
        waterPumpTimeStart = millis();                //Get current time stamp from millis() to make it loop.
        checkWaterFlowStart = millis();               //Get current time stamp from millis() to make it loop.
      }
      checkMoistureStart = millis();                  //Get current time stamp from millis() to make it loop.
    }

    //Stop water pump after it has run for a certain amount of time.
    unsigned long waterPumpTimeCurrent;

    if (waterPumpState == true) {
      waterPumpTimeCurrent = millis();                  //Get current time stamp from millis().
      if (waterPumpTimeCurrent - waterPumpTimeStart >= WATER_PUMP_TIME_PERIOD) { //Check if time period has elapsed.
        actionRegister = 8;                           //Register to print what action that is currently performed in the greenhouse program.
        waterPumpStop();                              //Time period has elapsed. Stop water pump (OFF).
        waterPumpEnabled = false;                     //Disable water pump from running until next time moisture value readout.
      }
    }

    //Check if water is being pumped when water pump is running by checking the water flow sensor. If not it will set an alarm.
    unsigned long checkWaterFlowCurrent;

    if (waterPumpState == true) {
      //checkWaterFlowCurrent = millis();                 //Get current time stamp of millis().
      //if (checkWaterFlowCurrent - checkWaterFlowStart >= CHECK_WATER_FLOW_PERIOD) {    //Check if time period has elapsed.
      //waterFlowCheck();                             //Time period has elapsed. Check water flow.
    }
  }
}

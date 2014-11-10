// Arduino pins
// Assignment of these pins (except 10-13/A0-A1 or I2C's SCL/SDA) can be re-configured here.
//
//  Arduino Due           || Teensy 3.1             || Teensy 3.0             || Arduino Pro Mini            || GR-KURUMI
#if defined (__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK20DX128__) || defined(__AVR_ATmega328P__) || defined(REL_GR_KURUMI)
//                           0;  // (Used by serial port)
//                           1;  // (Used by serial port)
const int SHUTTER_PIN      = 2;  // Interrupt pin w/o software debounce
//                           3;  // (Not in use)
const int IRRECV_PIN       = 4;  // IR remote controller
const int SWITCH0_PIN      = 5;  // Software debounced; ON-start ON-stop
const int SWITCH1_PIN      = 6;  // Software debounced; ON-start OFF-stop
//                           7;  // (Arduino: Not in use; GR-KURUMI: Used by I2C SCL)
//                           8;  // (Arduino: Not in use; GR-KURUMI: Used by I2C SDA)
const int PIR_PIN          = 9;  // Passive InfraRed motion sensor
// BEGIN Don't change the following pin allocations. These are used to control Herobus. 
const int I2CINT           = 10; // (SS)
const int TRIG             = 11; // (MOSI)
const int BPRDY            = 12; // (MISO) Pulled up by camera
//                           13; // (SCK) built-in LED
const int HBUSRDY          = A0; // (14)
const int PWRBTN           = A1; // (15) Pulled up by camera
// END Don't change.
//                           A2; // (16) (Not in use)
//                           A3; // (17) (Not in use)
//                           A4; // (18) (Arduino: Used by I2C SDA; GR-KURUMI: Not in use)
//                           A5; // (19) (Arduino: Used by I2C SCL; GR-KURUMI: Not in use) 
//                           A6; // (20) Analog only (Not in use)
const int LIGHT_SENSOR_PIN = A7; // (21) Analog only
//
//    Arduino Pro Micro / Arduino Leonardo (3.3V)
#elif defined(__AVR_ATmega32U4__)
const int SHUTTER_PIN      = 0;    // Interrupt pin w/o software debounce
//                           1;    // (Not in use)
//                           2;    // (Used by I2C SDA)
//                           3;    // (Used by I2C SCL)
const int IRRECV_PIN       = 4;    // (24 | A6) IR remote controller
const int SWITCH0_PIN      = 5;    // Software debounced; ON-start ON-stop
const int SWITCH1_PIN      = 6;    // (25 | A7) Software
//                           7;    // (Not in use)
const int LIGHT_SENSOR_PIN = 8;    // (26 | A8)
const int PIR_PIN          = 9;    // (27 | A9) Passive InfraRed motion sensor
// BEGIN Don't change the following pin allocations. These are used to control Herobus. 
const int I2CINT           = 10;   // (28 | A10)
//                           11;   //              (Arduino Pro Micro: No pin)
//                           12;   // (29 | A11)   (Arduino Pro Micro: No pin)
//                           13;   // built-in LED (Arduino Pro Micro: No pin, No LED)
const int TRIG             = MOSI; // (16)
const int BPRDY            = MISO; // (14) Pulled up by camera
//                           SCK;  // (15) (Not in use)
//                           SS;   // (17) RXLED   (Arduino Pro Micro: No pin)
const int HBUSRDY          = A0;   // (18)
const int PWRBTN           = A1;   // (19) Pulled up by camera
// END Don't change.
//                           A2;   // (20) (Not in use)
//                           A3;   // (21) (Not in use)
//                           A4;   // (22)         (Arduino Pro Micro: No pin)
//                           A5;   // (23)         (Arduino Pro Micro: No pin)
#else
#error CPU not supported
#endif

extern boolean ledState;
extern void ledOff();
extern void ledOn();
extern void setupLED();

// function prototypes
//   Arduino IDE doesn't need these prototypes but does Renesas Web Compiler
void resetI2C(void);
void setupShutter(void);
void setupSwitch(void);
void setupIRremote(void);
void setupLightSensor(void);
void setupPIRSensor(void);
void resetVMD(void);
void checkTimeAlarms(void);
void checkBacpacCommands(void);
void checkCameraCommands(void);
void checkSwitch(void);
void checkIRremote(void);
void checkLightSensor(void);
void checkPIRSensor(void);
void checkVMD(void);
// end of function prototypes

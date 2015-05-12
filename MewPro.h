// Arduino pins
// Assignment of these pins (except 10-13/A0-A1 or I2C's SCL/SDA) can be re-configured here.
//
//  Arduino Due           || Teensy 3.1             || Teensy 3.0             || Teensy LC             || Arduino Pro Mini            || GR-KURUMI
#if defined (__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK20DX128__) || defined(__MKL26Z64__) || defined(__AVR_ATmega328P__) || defined(REL_GR_KURUMI)
//                           0;  // (Used by Serial port RXI)
//                           1;  // (Used by Serial port TXO)
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
//                           0;    // (Used by Serial1 port RXI)
//                           1;    // (Used by Serial1 port TXO)
//                           2;    // (Used by I2C SDA)
//                           3;    // (Used by I2C SCL)
const int IRRECV_PIN       = 4;    // (24 | A6) IR remote controller
const int SWITCH0_PIN      = 5;    // Software debounced; ON-start ON-stop
const int SWITCH1_PIN      = 6;    // (25 | A7) Software debounced; ON-start OFF-stop
const int SHUTTER_PIN      = 7;    // Interrupt pin w/o software debounce
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
//
//    For ATtiny1634 core at https://github.com/SpenceKonde/arduino-tiny-841
#elif defined(__AVR_ATtiny1634__)
//                           0;    // ADC                  (Used by Serial port TXO)
//                           1;    // ADC                  (Used by Serial port RXI)
//                           2;    // ADC PWM
const int TRIG             = 3;    // ADC PWM
const int PWRBTN           = 4;    // ADC
const int I2CINT           = 5;    // ADC
//                           6;    // AIN1
//                           7;    // AIN0
//                           8;    // AREF
//                                 // GND
//                                 // VCC
//                                 // (9)  XTAL1
//                                 // (10) XTAL2
//                                 // (17) RESET
const int SHUTTER_PIN      = 11;   // ADC INT0             Interrupt pin w/o software debounce
//                           12;   // ADC             SCK  (Used by I2C SCL)
//                           13;   // ADC PWM              built-in LED (optional)
const int BPRDY            = 14;   // ADC PWM
const int HBUSRDY          = 15;   // ADC Serial1 TXO MISO
//                           16;   // ADC Serial1 RXI MOSI (Used by I2C SDA)
// the following are not supported
const int IRRECV_PIN       = 2;    // IR remote controller
const int SWITCH0_PIN      = 6;    // Software debounced; ON-start ON-stop
const int SWITCH1_PIN      = 7;    // Software debounced; ON-start OFF-stop
const int LIGHT_SENSOR_PIN = 8;    //
const int PIR_PIN          = 13;   // Passive InfraRed motion sensor
#define digitalPinToInterrupt(a) (0) // INT0
#else
#error CPU not supported
#endif

// commands need to be excuted in MewPro before sending to camera
const short int SET_CAMERA_3D_SYNCHRONIZE         = ('S' << 8) + 'Y';
const short int GET_CAMERA_INFO                   = ('c' << 8) + 'v';
const short int GET_CAMERA_SETTING                = ('t' << 8) + 'd';
const short int SET_CAMERA_SETTING                = ('T' << 8) + 'D';
const short int SET_CAMERA_VIDEO_OUTPUT           = ('V' << 8) + 'O';
const short int SET_CAMERA_AUDIOINPUTMODE         = ('A' << 8) + 'I';
const short int SET_CAMERA_USBMODE                = ('U' << 8) + 'M';
const short int SET_CAMERA_DATE_TIME              = ('T' << 8) + 'M';
// commands not relating to TD SET_CAMERA_SETTING
const short int GET_BACPAC_PROTOCOL_VERSION       = ('v' << 8) + 's';
const short int SET_BACPAC_DELETE_ALL             = ('D' << 8) + 'A';
const short int SET_BACPAC_DELETE_LAST            = ('D' << 8) + 'L';
const short int SET_BACPAC_FAULT                  = ('F' << 8) + 'N';
const short int SET_BACPAC_HEARTBEAT              = ('H' << 8) + 'B';
const short int SET_BACPAC_POWER_DOWN             = ('P' << 8) + 'W';
const short int SET_BACPAC_3D_SYNC_READY          = ('S' << 8) + 'R';
const short int SET_BACPAC_WIFI                   = ('W' << 8) + 'I';
const short int SET_BACPAC_SLAVE_SETTINGS         = ('X' << 8) + 'S';
const short int SET_BACPAC_SHUTTER_ACTION         = ('S' << 8) + 'H';
//
// commands relating to TD SET_CAMERA_SETTING
// buffer to store current camera settings
const int TD_BUFFER_SIZE                    = 0x29;
byte td[TD_BUFFER_SIZE];
// already read camera settings?
boolean tdDone = false;
//
// td[] meanings and associated bacpac command
const short int SET_BACPAC_DATE_TIME              = ('T' << 8) + 'M';
const int TD_DATE_TIME_year                 = 0x03; // year (0-99)    
const int TD_DATE_TIME_month                = 0x04; // month (1-12)
const int TD_DATE_TIME_day                  = 0x05; // day (1-31)
const int TD_DATE_TIME_hour                 = 0x06; // hour (0-23)
const int TD_DATE_TIME_minute               = 0x07; // minute (0-59)
const int TD_DATE_TIME_second               = 0x08; // second (0-59)
const short int SET_BACPAC_MODE                   = ('C' << 8) + 'M';
const int TD_MODE                           = 0x09;
const short int SET_BACPAC_PHOTO_RESOLUTION       = ('P' << 8) + 'R';
const int TD_PHOTO_RESOLUTION               = 0x0a;
const short int SET_BACPAC_VIDEORESOLUTION        = ('V' << 8) + 'R';
const int TD_VIDEORESOLUTION                = 0x0b; // (Defunct; always 0xff)
const short int SET_BACPAC_VIDEORESOLUTION_VV     = ('V' << 8) + 'V';
const int TD_VIDEORESOLUTION_VV             = 0x0c;
const short int SET_BACPAC_FRAMES_PER_SEC         = ('F' << 8) + 'S';
const int TD_FRAMES_PER_SEC                 = 0x0d;
const short int SET_BACPAC_FOV                    = ('F' << 8) + 'V';
const int TD_FOV                            = 0x0e;
const short int SET_BACPAC_EXPOSURE               = ('E' << 8) + 'X';
const int TD_EXPOSURE                       = 0x0f;
const short int SET_BACPAC_PHOTO_XSEC             = ('T' << 8) + 'I';
const int TD_PHOTO_XSEC                     = 0x10;
const short int SET_BACPAC_TIME_LAPSE             = ('T' << 8) + 'S';
const int TD_TIME_LAPSE                     = 0x11; // (Defunct; always 0)
const short int SET_BACPAC_BEEP_SOUND             = ('B' << 8) + 'S';
const int TD_BEEP_SOUND                     = 0x12;
const short int SET_BACPAC_NTSC_PAL               = ('V' << 8) + 'M';
const int TD_NTSC_PAL                       = 0x13;
const short int SET_BACPAC_ONSCREEN_DISPLAY       = ('D' << 8) + 'S';
const int TD_ONSCREEN_DISPLAY               = 0x14;
const short int SET_BACPAC_LEDBLINK               = ('L' << 8) + 'B';
const int TD_LEDBLINK                       = 0x15;
const short int SET_BACPAC_PHOTO_INVIDEO          = ('P' << 8) + 'N';
const int TD_PHOTO_INVIDEO                  = 0x16;
const short int SET_BACPAC_LOOPING_MODE           = ('L' << 8) + 'O';
const int TD_LOOPING_MODE                   = 0x17;
const short int SET_BACPAC_CONTINUOUS_SHOT        = ('C' << 8) + 'S';
const int TD_CONTINUOUS_SHOT                = 0x18;
const short int SET_BACPAC_BURST_RATE             = ('B' << 8) + 'U';
const int TD_BURST_RATE                     = 0x19;
const short int SET_BACPAC_PROTUNE_MODE           = ('P' << 8) + 'T';
const int TD_PROTUNE_MODE                   = 0x1a;
const short int SET_BACPAC_AUTO_POWER_OFF         = ('A' << 8) + 'O';
const int TD_AUTO_POWER_OFF                 = 0x1b;
const short int SET_BACPAC_WHITE_BALANCE          = ('W' << 8) + 'B';
const int TD_WHITE_BALANCE                  = 0x1c;
// 0x1d (reserved)
// 0x1e (reserved)
// 0x1f (reserved)
// 0x20 (reserved)
// 0x21 (reserved)
// 0x22 (reserved)
const short int SET_BACPAC_FLIP_MIRROR            = ('U' << 8) + 'P';
const int TD_FLIP_MIRROR                    = 0x23;
const short int SET_BACPAC_DEFAULT_MODE           = ('D' << 8) + 'M';
const int TD_DEFAULT_MODE                   = 0x24;
const short int SET_BACPAC_PROTUNE_COLOR          = ('C' << 8) + 'O';
const int TD_PROTUNE_COLOR                  = 0x25;
const short int SET_BACPAC_PROTUNE_GAIN           = ('G' << 8) + 'A';
const int TD_PROTUNE_GAIN                   = 0x26;
const short int SET_BACPAC_PROTUNE_SHARPNESS      = ('S' << 8) + 'P';
const int TD_PROTUNE_SHARPNESS              = 0x27;
const short int SET_BACPAC_PROTUNE_EXPOSURE_VALUE = ('E' << 8) + 'V';
const int TD_PROTUNE_EXPOSURE_VALUE         = 0x28;

#define MODE_VIDEO 0x00
#define MODE_PHOTO 0x01
#define MODE_BURST 0x02
#define MODE_TIMELAPSE 0x03
#define MODE_DUAL 0x08

const short int tdtable[] PROGMEM = {
  SET_BACPAC_MODE, // 0x09
  SET_BACPAC_PHOTO_RESOLUTION, // 0x0a
  SET_BACPAC_VIDEORESOLUTION, // 0x0b
  SET_BACPAC_VIDEORESOLUTION_VV, // 0x0c
  SET_BACPAC_FRAMES_PER_SEC, // 0x0d
  SET_BACPAC_FOV, // 0x0e
  SET_BACPAC_EXPOSURE, // 0x0f
  SET_BACPAC_PHOTO_XSEC, // 0x10
  SET_BACPAC_TIME_LAPSE, // 0x11
  SET_BACPAC_BEEP_SOUND, // 0x12
  SET_BACPAC_NTSC_PAL, // 0x13
  SET_BACPAC_ONSCREEN_DISPLAY, // 0x14
  SET_BACPAC_LEDBLINK, // 0x15
  SET_BACPAC_PHOTO_INVIDEO, // 0x16
  SET_BACPAC_LOOPING_MODE, // 0x17
  SET_BACPAC_CONTINUOUS_SHOT, // 0x18
  SET_BACPAC_BURST_RATE, // 0x19
  SET_BACPAC_PROTUNE_MODE, // 0x1a
  SET_BACPAC_AUTO_POWER_OFF, // 0x1b
  SET_BACPAC_WHITE_BALANCE, // 0x1c
  -1, // 0x1d (reserved)
  -1, // 0x1e (reserved)
  -1, // 0x1f (reserved)
  -1, // 0x20 (reserved)
  -1, // 0x21 (reserved)
  -1, // 0x22 (reserved)
  SET_BACPAC_FLIP_MIRROR, // 0x23
  SET_BACPAC_DEFAULT_MODE, // 0x24
  SET_BACPAC_PROTUNE_COLOR, // 0x25
  SET_BACPAC_PROTUNE_GAIN, // 0x26
  SET_BACPAC_PROTUNE_SHARPNESS, // 0x27
  SET_BACPAC_PROTUNE_EXPOSURE_VALUE, // 0x28
};

// .cpp file will be compiled separately
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
void setupGenlock(void);
void resetVMD(void);
void checkTimeAlarms(void);
void checkBacpacCommands(void);
void checkCameraCommands(void);
void checkSwitch(void);
void checkIRremote(void);
void checkLightSensor(void);
void checkPIRSensor(void);
void checkVMD(void);
void checkGenlock(void);
void startGenlock(void);
void stopGenlock(void);
// end of function prototypes

// MewPro
//
// The following small-factor micro processor boards are known to work with MewPro at least core functionalities and fit within the GoPro housing.
// Not all the sensors, however, are supported by each of them.
//
//   Arduino Pro Mini 328 3.3V 8MHz
//          w/ Arduino IDE 1.5.7+
//          if you have troubles in compiling unused or nonexistent libraries, simply comment out #include line as //#include (see Note* below)
//
//   Arduino Pro Micro - 3.3V 8MHz
//          w/ Arduino IDE 1.5.7+
//          if you have troubles in compiling unused or nonexistent libraries, simply comment out #include line as //#include (see Note* below)
//
//   Teensy 3.x / Teensy LC
//          [POWER SUPPLY: The VIN-VUSB pad connection on the bottom side of Teensy 3.x/LC needs to be cut.]
//          To compile the code with Teensy 3.x/LC:
//          1. use Arduino IDE 1.0.6+ and Teensyduino 1.20+
//          2. comment out all unused #include as //#include (see Note* below)
//
//   (Note*: There is an infamous Arduino IDE's preprocessor bug (or something) that causes to ignore #ifdef/#else/#endif directives and forces 
//    to compile unnecessary libraries.)
//
//   Intel Edison
//          Since Intel Edison can not work as I2C slave, it requires an I2C proxy.
//
//   GR-KURUMI
//          [POWER SUPPLY: The dummy resistor soldered on JP1 of the MewPro board needs replacement w/ a general purpose diode of >100mA w/ dropoff voltage 1V
//           (eg. Bourns S0180); Anode should be located on the Herobus side and Cathode on the Arduino side.]
//          To compile the code with GR-KURUMI using Renesas web compiler http://www.renesas.com/products/promotion/gr/index.jsp#cloud :
//          1. open a new project with the template GR-KURUMI_Sketch_V1.04.zip.
//          2. create a folder named MewPro and upload all the files there.
//          3. at project's home directory, replace all the lines of gr_scketch.cpp by the following code (BEGIN / END lines should be excluded).
/* BEGIN copy
#include <RLduino78.h>
#include "MewPro/MewPro.ino"
#include "MewPro/a_Queue.ino"
#include "MewPro/b_TimeAlarms.ino"
#include "MewPro/c_I2C.ino"
#include "MewPro/d_BacpacCommands.ino"
#include "MewPro/e_Shutter.ino"
#include "MewPro/f_Switch.ino"
#include "MewPro/g_IRremote.ino"
#include "MewPro/h_LightSensor.ino"
#include "MewPro/i_PIRsensor.ino"
#include "MewPro/j_VideoMotionDetect.ino"
#include "MewPro/k_Genlock.ino"
END copy */
//

//   Copyright (c) 2014-2016 orangkucing
//
// MewPro firmware version string for maintenance
#define MEWPRO_FIRMWARE_VERSION "2016020800"

//
#include <Arduino.h>
#include <EEPROM.h>
#include "MewPro.h"

// enable console output
// set false if this is MewPro #0 of dual dongle configuration
boolean debug = true;

//////////////////////////////////////////////////////////
// Options:
//   Choose either "#define" to use or "#undef" not to use. 
//   if #define then don't forget to remove // before //#include

//********************************************************
// a_TimeAlarms: MewPro driven timelapse
#undef  USE_TIME_ALARMS
// Time and TimeAlarms libraries are downloadable from
//   http://www.pjrc.com/teensy/td_libs_Time.html
//   and http://www.pjrc.com/teensy/td_libs_TimeAlarms.html
// In order to compile the code on Pro Mini 328, find the following and edit the first line of them
//   #if defined(__AVR__)
//   #include <avr/pgmspace.h>
//   #else
// to
//   #if defined(__AVR__) && !defined(__AVR_ATmega328P__)
//   #include <avr/pgmspace.h>
//   #else
// appeared in Documents/Arduino/libraries/Time/DateStrings.cpp
//#include <Time.h> // *** please comment out this line if USE_TIME_ALARMS is not defined ***
//#include <TimeAlarms.h> // *** please comment out this line if USE_TIME_ALARMS is not defined ***

//********************************************************
// c_I2C: I2C interface (THIS PART CAN'T BE OPTED OUT)
// 
// Note: in order to use MewPro reliably, THE FOLLOWING MODIFICATIONS TO STANDARD ARDUINO LIBRARY SOURCE IS
// STRONGLY RECOMMENDED:
//
// Arduino Pro Mini / Arduino Pro Micro
//     1. hardware/arduino/avr/libraries/Wire/Wire.h
//            old: #define BUFFER_LENGTH 32                        -->   new: #define BUFFER_LENGTH 64
//     2. hardware/arduino/avr/libraries/Wire/utility/twi.h
//            old: #define TWI_BUFFER_LENGTH 32                    -->   new: #define TWI_BUFFER_LENGTH 64
// Arduino Due
//     hardware/arduino/sam/libraries/Wire/Wire.h
//            old: #define BUFFER_LENGTH 32                        -->   new: #define BUFFER_LENGTH 64
#include <Wire.h> // *** please comment out this line if __MK20DX256__ or __MK20DX128__ or __MKL26Z64__ or __AVR_ATtiny1634__ is defined ***
#if BUFFER_LENGTH < 64
#error Please modify Arduino Wire library source code to increase the I2C buffer size
#endif
//
// Teensy 3.0 or 3.1 or LC
//#include <i2c_t3.h> // *** please comment out this line if __MK20DX256__ and __MK20DX128__ and __MKL26Z64__ are not defined ***
//
// ATtiny1634 core https://github.com/SpenceKonde/arduino-tiny-841
//    WireS library is downloadable from https://github.com/orangkucing/WireS
//#include <WireS.h> // *** please comment out this line if __AVR_ATtiny1634__ is not defined ***

#undef  USE_I2C_PROXY // define if using I2C proxy and this CPU acts as I2C master

//********************************************************
// e_Shutters: A remote shutters without contact bounce or chatter
#undef  USE_SHUTTER

//********************************************************
// f_Switches: One or two mechanical switches
#undef  USE_SWITCHES

//********************************************************
// g_IRremote: IR remote controller
#undef  USE_IR_REMOTE
// IRremote2 is downloadable from https://github.com/enternoescape/Arduino-IRremote-Due
// (this works not only on Due but also on Pro Mini etc.)
//#include <IRremote2.h> // *** please comment out this line if USE_IR_REMOTE is not defined ***

//********************************************************
// h_LightSensor: Ambient light sensor
#undef  USE_LIGHT_SENSOR

//********************************************************
// i_PIRsensor: Passive InfraRed motion sensor
#undef  USE_PIR_SENSOR

//********************************************************
// j_VideoMotionDetect: Video Motion Detector
//   Video motion detect consumes almost all the dynamic memory. So if you want to use this then #undef all options above.
#undef  USE_VIDEOMOTION
// The part of code utilizes the following library except GR-KURUMI. Please download and install:
//   https://github.com/orangkucing/analogComp
//#include "analogComp.h" // *** please comment out this line if USE_VIDEOMOTION is not defined or GR-KURUMI ***

//********************************************************
// k_Genlock: Generator Lock
//   Note: MewPro #0 in dual dongle configuration should always boolean debug = false;
#undef  USE_GENLOCK

// it is better to define this when RXI is connected to nothing (eg. MewPro #0 of Genlock system)
#undef  UART_RECEIVER_DISABLE

// end of Options
//////////////////////////////////////////////////////////

// if __AVR_ATmega32U4__ (Leonardo or Pro Micro) then use Serial1 (TTL) instead of Serial (USB) to communicate with genlock dongle
#if defined(USE_GENLOCK) && defined(__AVR_ATmega32U4__)
#define Serial Serial1
#endif

boolean lastHerobusState = LOW;  // Will be HIGH when camera attached.
int eepromId = 0;

void userSettings()
{
  // This function is called once after camera boot.
  // you can set put any camera commands here. For example:
  // queueIn("AI1");
  // queueIn("TI5");
}

void setup()
{
  // Remark. Arduino Pro Mini 328 3.3V 8MHz is too slow to catch up with the highest 115200 baud.
  //     cf. http://forum.arduino.cc/index.php?topic=54623.0
  // Set 57600 baud or slower.
  Serial.begin(57600);
#ifdef UART_RECEIVER_DISABLE
#ifndef __AVR_ATmega32U4__
  UCSR0B &= (~_BV(RXEN0));
#else
  UCSR1B &= (~_BV(RXEN1));
#endif
#endif

  setupShutter();
  setupSwitch();
  setupIRremote();
  setupLightSensor();
  setupPIRSensor();
  setupGenlock();

  setupLED(); // onboard LED setup 
  pinMode(BPRDY, OUTPUT); digitalWrite(BPRDY, LOW);    // Show camera MewPro attach. 

  // don't forget to switch pin configurations to INPUT.
  pinMode(I2CINT, INPUT);  // Teensy: default disabled
  pinMode(HBUSRDY, INPUT); // default: analog input
  digitalWrite(PWRBTN, HIGH);
  pinMode(PWRBTN, OUTPUT);  // default: analog input
}

void loop() 
{
  // Attach or detach bacpac
  //if (digitalRead(HBUSRDY) == HIGH) {
  if ((PINC & _BV(0)) == 1) { // speed up!
    if (lastHerobusState != HIGH) {
#if !defined(USE_I2C_PROXY)
      pinMode(I2CINT, OUTPUT); digitalWrite(I2CINT, HIGH);
#endif
      lastHerobusState = HIGH;
      if (eepromId == 0) {
        isMaster(); // determine master/slave and resetI2C()
      } else {
        resetI2C();
      }
    }
  } else {
    if (lastHerobusState != LOW) {
      pinMode(I2CINT, INPUT);
      lastHerobusState = LOW;
    }
  }
  checkTimeAlarms();
  checkBacpacCommands();
  checkCameraCommands();
  checkSwitch();
  checkIRremote();
  checkLightSensor();
  checkPIRSensor();
  checkVMD();
  checkGenlock();
}


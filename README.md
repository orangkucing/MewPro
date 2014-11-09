#MewPro

Arduino BacPac™ for GoPro Hero 3+ Black: GoPro can be controlled by Arduino Pro Mini attached on Herobus.

Resources:
* Introduction to MewPro: [http://mewpro.cc/?p=226]
* Schematic Drawing: [http://mewpro.cc/?p=204]
* List of GoPro Serial Commands: [http://mewpro.cc/2014/10/14/list-of-i%C2%B2c-commands/]
* Herobus Pinout of GoPro Hero 3+ Black: [http://mewpro.cc/?p=207]

------

###How To Compile
The following small-factor microcontroller boards are known to work with MewPro at least core functionalities and fit within the GoPro housing. Not all the sensors, however, are supported by each of them.

* Arduino Pro Mini 328 3.3V 8MHz (also overclocked version of 3.3V 16MHz)
  - w/ Arduino IDE 1.5.7+
  - if you have troubles on compiling unused or nonexistent libraries, simply comment out #include line as //#include (see Note* below)

* Arduino Pro Micro - 3.3V 8MHz (5V 16MHz)
  - w/ Arduino IDE 1.5.7+
  - [POWER SUPPLY: 5V 16MHz version needs removal of the solder jumper on pads marked SJ1.]
  - if you have troubles in compiling unused or nonexistent libraries, simply comment out #include line as //#include (see Note* below)

* Teensy 3.x
  - [POWER SUPPLY: The VIN-VUSB pad connection on the bottom side of Teensy 3.x needs to be cut.]
  - To compile the code with Teensy 3.x:
  1. use Arduino IDE 1.0.6+ and Teensyduino 1.20+
  2. comment out all unused #include as //#include (see Note* below)

(Note*: There is an infamous Arduino IDE's preprocessor bug (or something) that causes to ignore #ifdef/#else/#endif directives and forces to compile unnecessary libraries.)

* GR-KURUMI
  - [POWER SUPPLY: The dummy resistor soldered on JP1 of the MewPro board needs replacement w/ a general purpose diode of >100mA w/ dropoff voltage 1V (eg. Bourns S0180); Anode should be located on the Herobus side and Cathode on the Arduino side.]
  - To compile the code with GR-KURUMI using (Renesas web compiler)[http://www.renesas.com/products/promotion/gr/index.jsp#cloud] :
  1. open a new project with the template GR-KURUMI_Sketch_V1.04.zip
  2. create a folder named MewPro and upload all the files there.
  3. at project's home directory, replace all the lines of gr_scketch.cpp by the following code:
```
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
```

------

###Serial Line Commands
By default MewPro is configured to use the serial line for controlling GoPro. All the commands are listed at https://gist.github.com/orangkucing/45dd2046b871828bf592#file-gopro-i2ccommands-md . You can simply type a command string to the serial console followed by a return; for example,

+ `PW0` : shutdown GoPro
+ `TM0E0A0D090F00` : set GoPro clock to 2014-10-13 09:15:00 (hexadecimal of YYYY-MM-DD hh:mm:ss)
+ `SY1` : shutter (camera mode) or start record (video mode)
+ `SY0` : stop record (video mode)

Almost all listed commands that have a label named SET_CAMERA_xxx are usable. Moreover two special command are implemented in MewPro:

+ `@` : GoPro power on
+ `!` : toggle the role of MewPro (slave -> master or master -> slave)

Also the command `!` can be used to write the necessary bytes to onboard blank/new I²C EEPROM chip: In order to work as a fake Dual Hero Bacpac™, MewPro's I²C EEPROM must contain such info.

------

###Sensors
The code here includes examples to support various sensors:

+ b_TimeAlarms.ino : Using RTC (real time clock) in GoPro camera body.
+ e_Shutter.ino : Using external shutters such as CANON Timer Remote Controller TC-80N3.
+ f_Switch.ino : Using mechanical switches such as push buttons or reed switches
+ g_IRremote.ino : Using IR (infra red) remote controllers such as DFRobot Digital IR Receiver Module (Arduino), DFR0094.
+ h_LightSensor.ino : Using Microsemi Visible Light Sensor LX1971 and green laser pointers.
+ i_PIRsensor.ino : Using Parallax PIR sensors.
+ j_VideoMotionDetect.ino : Using Intersil Sync Separator EL1883 and a bulk motion detect algorithm.

If you like to use these functions please refer the corresponding .ino files.

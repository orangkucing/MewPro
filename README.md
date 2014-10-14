###MewPro
======
Arduino BacPac™ for GoPro Hero 3+ Black: GoPro can be controlled by Arduino Pro Mini attached on Herobus.


###How To Compile
======
The following microcontroller boards are known to work with MewPro at least core functionalities. Not all the sensors, however, are supported by each of them.

*Arduino Pro Mini 328 3.3V 8MHz and its clones
*Arduino Pro 328 3.3V 8MHz
*Arduino Due
  -w/ Arduino IDE 1.5.7 beta
  -if you have troubles on compiling unused or nonexistent libraries, simply comment out #include line as //#include (see Note below)

*Teensy 3.1
  -To compile the code with Teensy 3.1:
  -1. use Arduino IDE 1.0.6 and Teensyduino 1.20-rc5
  -2. comment out all unused #include as //#include (see Note below)

(Note: There is an infamous Arduino IDE's preprocessor bug (or something) that causes to ignore #ifdef/#else/#endif directives and forces to compile unnecessary libraries.)

*GR-KURUMI
  -To compile the code with GR-KURUMI using Renesas web compiler     http://www.renesas.com/products/promotion/gr/index.jsp#cloud :
  -1. open a new project with the template GR-KURUMI_Sketch_V1.04.zip
  -2. create a folder named MewPro and upload all the files there.
  -3. at project's home directory, replace all the lines of gr_scketch.cpp by the following code (BEGIN / END lines should be excluded).

###Sensors
======
By default MewPro is configured to use the serial line for controlling GoPro. All the commands are listed at https://gist.github.com/orangkucing/45dd2046b871828bf592#file-gopro-i2ccommands-md . You can simply type a command string to the serial console followed by a return; for example,

+`PW0` : shutdown GoPro
+`TM0E0A0D090F00` : set GoPro clock to 2014-10-13 09:15:00 (hexadecimal of YYYY-MM-DD hh:mm:ss)
+`SY1` : shutter (camera mode) or start record (video mode)
+`SY0` : stop record (video mode)

Almost all listed commands that have a label named SET_CAMERA_xxx are usable. Moreover two special command are implemented in MewPro:

+`@` : GoPro power on
+`!` : toggle the role of MewPro (slave -> master or master -> slave)

Also the command `!` can be used to write the necessary bytes to onboard blank/new I²C EEPROM chip: In order to work as a fake Dual Hero Bacpac™, MewPro's I²C EEPROM must contain such info.

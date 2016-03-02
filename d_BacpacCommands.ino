// bacpac commands
//
void emulateDetachBacpac()
{
  // to exit 3D mode, emulate detach and attach bacpac
  //
  // detach bacpac
  pinMode(BPRDY, INPUT);
  delay(1000);
  // attach bacpac again
  pinMode(BPRDY, OUTPUT);
  digitalWrite(BPRDY, LOW);
}

// what does this mean? i have no idea...
const unsigned char validationString[19] PROGMEM = { 18, 0, 0, 3, 1, 0, 1, 0x3f, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

void bacpacCommand()
{
  int command = (RECV(1) << 8) + RECV(2);
  switch (command) {
  case GET_BACPAC_PROTOCOL_VERSION: // vs
    ledOff();
#ifdef USE_GENLOCK
    if (1) { // send to Dongle
      Serial.println("");
      Serial.println('@');  // power on
    }
    tdDone = true;
#endif
    while (digitalRead(I2CINT) != HIGH) { // wait until camera pullups I2CINT
      ;
    }
    memcpy_P(buf, validationString, sizeof validationString);
    SendBufToCamera();
    delay(200); // need a short delay the validation string to be read by camera
    queueIn(F("cv"));
    break;
  case SET_BACPAC_DELETE_ALL: // DA
    break;
  case SET_BACPAC_DELETE_LAST: // DL
    break;
  case SET_BACPAC_FAULT: // FN
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (RECV(3) == 0x0c) {
//      queueIn(F("XS1"));
    }
    break;
  case SET_BACPAC_HEARTBEAT: // HB
#ifdef USE_GENLOCK
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (isMaster()) {
//         baterry:remaining:photos:seconds:videos:media:
//         03      FFFF      0000   FFFF    0000   00    FFFFFF
//      queueIn(F("XS0303FFFF0000FFFF000000FFFFFF")); // dummy  
    } else // fall down
#endif
    if (!tdDone) {
      emulateDetachBacpac();
      recvb = recve = 0; // clear I2C buffer
      tdDone = true;
      return;
    }
    break;
  case SET_BACPAC_POWER_DOWN: // PW
    tdDone = false;
#ifdef USE_GENLOCK
    if (1) { // send to Dongle
      Serial.println(F("PW00"));
      Serial.flush();
    }
#endif
    recvb = recve = 0; // clear I2C buffer
    return;
  case SET_BACPAC_3D_SYNC_READY: // SR
    switch (RECV(3)) {
    case 0: // CAPTURE_STOP
      stopGenlock();
      ledOff();
      break;
    case 1: // CAPTURE_START
      ledOn();
      // fall down
    case 2: // CAPTURE_INTERMEDIATE (PES only)
      startGenlock();
      break;
    case 3: // PES interim capture complete
      switch (td[TD_MODE]) {
      case MODE_VIDEO:
      case MODE_BURST:
      case MODE_PHOTO:
        stopGenlock();
        ledOff();
        break;
      default:
        break;
      }
      break;
    }
    break;
  case SET_BACPAC_WIFI: // WI
    break;
  case SET_BACPAC_SLAVE_SETTINGS: // XS
#ifndef USE_GENLOCK
    // every second message will be off if we send "XS0" here
    queueIn(F("XS0"));
    if (debug) {
      char tmp[13];
      // battery level: 0-3 (4 if charging)
      Serial.print(F(" batt_level:")); Serial.print(RECV(4));
      // photos remaining
      Serial.print(F(" remaining:")); Serial.print((RECV(5) << 8) + RECV(6));
      // photos on microSD card
      Serial.print(F(" photos:")); Serial.print((RECV(7) << 8) + RECV(8));
      // video time remaining (minutes)
      if ((RECV(9) << 8) + RECV(10) == 0) { // GoPro firmware bug!
        Serial.print(F(" minutes:")); Serial.print(F("unknown"));
      } else {
        Serial.print(F(" minutes:")); Serial.print((RECV(9) << 8) + RECV(10));
      }
      // videos on microSD card
      Serial.print(F(" videos:")); Serial.print((RECV(11) << 8) + RECV(12));
      // maximum file size (4GB if FAT32, 0 means infinity if exFAT)
      // if one video file exceeds the limit then GoPro will divide it into smaller files automatically
      Serial.print(' ');
      printHex(RECV(13), false);
      Serial.print(F("GB "));
      printHex(RECV(14), false);
      printHex(RECV(15), false);
      printHex(RECV(16), false);
      Serial.println("");
    }
 #endif
    break;
  case SET_BACPAC_SHUTTER_ACTION: // SH
    // shutter button of master is pressed
    buf[0] = 3; buf[1] = 'S'; buf[2] = 'Y'; buf[3] = RECV(3);
    SendBufToCamera();
#ifdef USE_GENLOCK
    timelapse = 0;  
    if (td[TD_MODE] == MODE_TIMELAPSE && buf[3] == 1) {
        timelapse = (unsigned long)td[TD_PHOTO_XSEC] * 1000;
        timelapse -= 10; // margin
    }
#endif
    break;
  case SET_BACPAC_DATE_TIME: // TM
    for (int i = 0; i < 6; i++) {
      td[TD_DATE_TIME_year + i] = RECV(TD_DATE_TIME_year + i);
    }
    _setTime();
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    break;
  default:
#ifdef USE_GENLOCK
    // other commands are listed in tdtable[]
    for (int offset = 0x09; offset < TD_BUFFER_SIZE; offset++) {
      if (pgm_read_word(tdtable + offset - 0x09) == command) {
        buf[0] = 1; buf[1] = 0; // Dual Hero doesn't understand each command
        SendBufToCamera();
        queueIn(F("td")); // let camera report setting in full
        break;
      }
    }
#endif
    break;
  }
  recvb = (recvb + (recv[recvb] & 0x7F) + 1) % MEWPRO_BUFFER_LENGTH;
}

// dummy setting: should be overridden soon
const char tmptd[TD_BUFFER_SIZE] PROGMEM = {
  // default mode below will be overwritten so don't worry about the detailed settings here
  // NOTE: TD_MODE must be equal to TD_DEFAULT_MODE at camera boot
#define MODE_DEFAULT MODE_VIDEO
  0x28, 'T', 'D', 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, MODE_DEFAULT, 0x05, 0xff, 0x0a, 0x07, 0x00, 0x00, 
  0x02, 0x00, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00,
  0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, MODE_DEFAULT, 0x00, 0x00, 0x00,
  0x0a,
};

void checkBacpacCommands()
{
#ifdef USE_I2C_PROXY
  if (digitalRead(I2CINT) == LOW) {
    receiveHandler();
  }
#endif
  if (recvb != recve) {
    waiting = false;
    _printInput();
    if (!(RECV(0) & 0x80)) {// information bytes
      if (RECV(0) == 0x25) {
        // initialize bacpac
#ifndef USE_GENLOCK
        if (!tdDone) { // this is first time to see vs
#else
        if (1) {
          if (isMaster()) {
            __debug(F("master bacpac and use genlock"));
//            resetVMD();
//            queueIn(F("VO1")); // SET_CAMERA_VIDEO_OUTPUT to herobus
              userSettings();
          }
#endif
          // td lets camera into 3D mode
          // camera will send HBFF
          queueIn(F("td"));
          // unless this is master and USE_GENLOCK, after receiving HBFF we are
          // going to emulate detach bacpac
        } else {
          // this is second time to see vs, i.e.,
          //   > vs
          //   < td
          //   > HB FF
          // (emulate detach bacpac)
          //   > vs
          if (isMaster()) {
            __debug(F("master bacpac and not use genlock"));
            resetVMD();
            queueIn(F("VO1")); // SET_CAMERA_VIDEO_OUTPUT to herobus
            userSettings(); 
          } else {
#ifdef USE_GENLOCK
            __debug(F("slave bacpac and use genlock (not supported yet)"));
            memcpy_P(buf, tmptd, TD_BUFFER_SIZE);
            memcpy_P(td, tmptd, TD_BUFFER_SIZE);
            SendBufToCamera();
#else
            __debug(F("slave bacpac and not use genlock"));
            queueIn(F("XS1"));
#endif
            userSettings();
          }
        }
      } else if (RECV(0) == 0x27) {
        // Usual packet length (RECV(0)) is 1 or 2.
        // Packet length 0x27 does not exist but SMARTY_START
        for (int i = 0; i < TD_BUFFER_SIZE - 1; i++) {
          td[i + 1] = RECV(i);
        }
        td[0] = TD_BUFFER_SIZE-1; td[1] = 'T'; td[2] = 'D'; // get ready to submit to slave
#ifdef USE_GENLOCK
        if (isMaster()) {
          queueIn(F("FN0C")); // emulate slave ready
          // send camera config to master dongle
          // Upside is always up
          td[TD_FLIP_MIRROR] = 1;
          //
          Serial.print(F("TD"));
          for (int i = 3; i < TD_BUFFER_SIZE; i++) {
            printHex(td[i], true);
          }
          Serial.println("");
        }
#else
        _setTime();
#endif
      } else {
        ; // do nothing
      }
      recvb = (recvb + (recv[recvb] & 0x7F) + 1) % MEWPRO_BUFFER_LENGTH;
    } else { 
      bacpacCommand();
    }
  }
}

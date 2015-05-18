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
  int command = (recv[1] << 8) + recv[2];
  switch (command) {
  case GET_BACPAC_PROTOCOL_VERSION: // vs
    ledOff();
    memcpy_P(buf, validationString, sizeof validationString);
    SendBufToCamera();
#ifdef USE_GENLOCK
    if (1) { // send to Dongle
      Serial.println("");
      Serial.println('@');  // power on
      Serial.flush();
    }
#endif
    delay(200); // need a short delay the validation string to be read by camera
    queueIn(F("cv"));
    return;
  case SET_BACPAC_DELETE_ALL: // DA
    return;
  case SET_BACPAC_DELETE_LAST: // DL
    return;
  case SET_BACPAC_FAULT: // FN
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (recv[3] == 0x0c) {
//      queueIn(F("XS1"));
    }
    return;
  case SET_BACPAC_HEARTBEAT: // HB
#ifdef USE_GENLOCK
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (isMaster()) {
//         baterry:remaining:photos:seconds:videos:media:
//         03      FFFF      0000   FFFF    0000   00    FFFFFF
//      queueIn(F("XS0303FFFF0000FFFF000000FFFFFF")); // dummy
      return;   
    }
#endif
    if (!tdDone) {
      emulateDetachBacpac();
    }
    tdDone = true;
    return;
  case SET_BACPAC_POWER_DOWN: // PW
    tdDone = false;
#ifdef USE_GENLOCK
    if (1) { // send to Dongle
      Serial.println(F("PW00"));
      Serial.flush();
    }
#endif
    return;
  case SET_BACPAC_3D_SYNC_READY: // SR
    switch (recv[3]) {
    case 0: // CAPTURE_STOP
      stopGenlock();
      break;
    case 1: // CAPTURE_START
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
    return;
  case SET_BACPAC_WIFI: // WI
    return;
  case SET_BACPAC_SLAVE_SETTINGS: // XS
#ifndef USE_GENLOCK
    // every second message will be off if we send "XS0" here
    queueIn(F("XS0"));
    if (debug) {
      char tmp[13];
      // battery level: 0-3 (4 if charging)
      Serial.print(F(" batt_level:")); Serial.print(recv[4]);
      // photos remaining
      Serial.print(F(" remaining:")); Serial.print((recv[5] << 8) + recv[6]);
      // photos on microSD card
      Serial.print(F(" photos:")); Serial.print((recv[7] << 8) + recv[8]);
      // video time remaining (minutes)
      if ((recv[9] << 8) + recv[10] == 0) { // GoPro firmware bug!
        Serial.print(F(" minutes:")); Serial.print(F("unknown"));
      } else {
        Serial.print(F(" minutes:")); Serial.print((recv[9] << 8) + recv[10]);
      }
      // videos on microSD card
      Serial.print(F(" videos:")); Serial.print((recv[11] << 8) + recv[12]);
      // maximum file size (4GB if FAT32, 0 means infinity if exFAT)
      // if one video file exceeds the limit then GoPro will divide it into smaller files automatically
      Serial.print(' ');
      printHex(recv[13], false);
      Serial.print(F("GB "));
      printHex(recv[14], false);
      printHex(recv[15], false);
      printHex(recv[16], false);
      Serial.println("");
    }
 #endif
    return;
  case SET_BACPAC_SHUTTER_ACTION: // SH
    // shutter button of master is pressed
#ifdef USE_GENLOCK
    if (1) { // send to Dongle
      Serial.print(F("SH"));
      printHex(recv[3], true);
      Serial.println("");
      Serial.flush();
    }
#endif
    buf[0] = 3; buf[1] = 'S'; buf[2] = 'Y'; buf[3] = recv[3];
    SendBufToCamera();
    return;
  case SET_BACPAC_DATE_TIME: // TM
    memcpy((char *)td+TD_DATE_TIME_year, (char *)recv+TD_DATE_TIME_year, 6);
    _setTime();
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    return;
  default:
    break;
  }
#ifdef USE_GENLOCK
  // other commands are listed in tdtable[]
  for (int offset = 0x09; offset < TD_BUFFER_SIZE; offset++) {
    if (pgm_read_word(tdtable + offset - 0x09) == command) {
      buf[0] = 1; buf[1] = 0; // Dual Hero doesn't understand each command
      SendBufToCamera();
      queueIn(F("td")); // let camera report setting in full
      return;
    }
  }
#endif
}

// dummy setting: should be overridden soon
const char tmptd[TD_BUFFER_SIZE] PROGMEM = {
  0x28, 'T', 'D', 0x0f, 0x01, 0x12, 0x04, 0x0d, 0x33, MODE_PHOTO,
  0x05, 0xff, 0x03, 0x07, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00,
  0x01, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0a, };

void checkBacpacCommands()
{
#ifdef USE_I2C_PROXY
  if (digitalRead(I2CINT) == LOW) {
    receiveHandler();
  }
#endif
  if (recvq) {
    waiting = false;
    _printInput();
    if (!(recv[0] & 0x80)) {// information bytes
      if (recv[0] == 0x25) {
        // initialize bacpac
        if (!tdDone) { // this is first time to see vs
#ifdef USE_GENLOCK
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
      } else if (recv[0] == 0x27) {
        // Usual packet length (recv[0]) is 0 or 1.
        // Packet length 0x27 does not exist but SMARTY_START
        memcpy((char *)td+1, recv, TD_BUFFER_SIZE-1);
        td[0] = TD_BUFFER_SIZE-1; td[1] = 'T'; td[2] = 'D'; // get ready to submit to slave
#ifdef USE_GENLOCK
        if (isMaster()) {
          queueIn(F("FN0C")); // emulate slave ready
          // send camera config to master dongle
          Serial.print(F("TD"));
          for (int i = 3; i < TD_BUFFER_SIZE; i++) {
            printHex(td[i], true);
          }
          Serial.println("");
          Serial.flush();
        }
#else
        _setTime();
        setupTimeAlarms();
#endif
      } else {
        ; // do nothing
      }
    } else { 
        bacpacCommand();
    }
    recvq = false;
  }
}

// bacpac commands
//
boolean powerOnAtCameraMode = false;

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
unsigned char validationString[19] = { 18, 0, 0, 3, 1, 0, 1, 0x3f, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

void bacpacCommand()
{
  int command = (recv[1] << 8) + recv[2];
  switch (command) {
  case GET_BACPAC_PROTOCOL_VERSION: // vs
    ledOff();
    memcpy(buf, validationString, sizeof validationString);
    SendBufToCamera();
    delay(100); // need a short delay the validation string to be read by camera
    queueIn("cv");
    return;
  case SET_BACPAC_DELETE_ALL: // DA
    return;
  case SET_BACPAC_DELETE_LAST: // DL
    return;
  case SET_BACPAC_FAULT: // FN
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (recv[3] == 0x0c) {
//      queueIn("XS1");
    }
    return;
  case SET_BACPAC_HEARTBEAT: // HB
#ifdef USE_GENLOCK
    buf[0] = 1; buf[1] = 0; // "ok"
    SendBufToCamera();
    if (isMaster()) {
//         baterry:remaining:photos:seconds:videos:media:
//         03      FFFF      0000   FFFF    0000   00    FFFFFF
//      queueIn("XS0303FFFF0000FFFF000000FFFFFF"); // dummy
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
    Serial.println("PW0");
    Serial.flush();
#endif
    return;
  case SET_BACPAC_3D_SYNC_READY: // SR
    switch (recv[3]) {
    case 0: // CAPTURE_STOP
      stopGenlock();
      break;
    case 1: // CAPTURE_START
      if (powerOnAtCameraMode) { // powerOnAtCameraMode is always false when USE_GENLOCK
        delay(500); // delay for LED light to be visible
        ledOff();
      } else {
        startGenlock();
      }
      break;
    case 2: // CAPTURE_INTERMEDIATE (PES only)
    case 3: // PES interim capture complete
      stopGenlock();
      ledOff();
      break;
    }
    return;
  case SET_BACPAC_WIFI: // WI
    return;
  case SET_BACPAC_SLAVE_SETTINGS: // XS
#ifndef USE_GENLOCK
    if ((recv[9] << 8) + recv[10] == 0) {
      powerOnAtCameraMode = true;
    }
    // every second message will be off if we send "XS0" here
    queueIn("XS0");
    if (debug) {
      char tmp[13];
      // battery level: 0-3 (4 if charging)
      Serial.print(F(" batt_level:")); Serial.print(recv[4]);
      // photos remaining
      Serial.print(F(" remaining:")); Serial.print((recv[5] << 8) + recv[6]);
      // photos on microSD card
      Serial.print(F(" photos:")); Serial.print((recv[7] << 8) + recv[8]);
      // video time remaining (minutes)
      Serial.print(F(" minutes:")); Serial.print((recv[9] << 8) + recv[10]);
      // videos on microSD card
      Serial.print(F(" videos:")); Serial.print((recv[11] << 8) + recv[12]);
      // maximum file size (4GB if FAT32, 0 means infinity if exFAT)
      // if one video file exceeds the limit then GoPro will divide it into smaller files automatically
      sprintf(tmp, " %02xGB %02x%02x%02x", recv[13], recv[14], recv[15], recv[16]);
      Serial.println(tmp);
    }
 #endif
    return;
  case SET_BACPAC_SHUTTER_ACTION: // SH
    // shutter button of master is pressed
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
  // other commands are listed in tdtable[]
  for (int offset = 0x09; offset < TD_BUFFER_SIZE; offset++) {
    if (tdtable[offset - 0x09] == command) {
      td[offset] = recv[3];
      buf[0] = 2; buf[1] = 1; buf[2] = recv[3];
      SendBufToCamera();
#ifdef USE_GENLOCK
      if (isMaster()) {
        // send received command to master dongle
        Serial.print((char)recv[1]);
        Serial.print((char)recv[2]);
        {
          char tmp[3];
          sprintf(tmp, "%02X", recv[3]);
          Serial.print(tmp);
        }
        Serial.println("");
        Serial.flush();
      }
#endif      
      return;
    }
  }
}

void checkBacpacCommands()
{
  if (recvq) {
    _printInput();
    if (!(recv[0] & 0x80)) {// information bytes
      if (recv[0] == 0x25) {
        // initialize bacpac
        if (!tdDone) { // this is first time to see vs
#ifdef USE_GENLOCK
          if (isMaster()) {
            __debug(F("master bacpac and use genlock"));
//            resetVMD();
//            memcpy(buf, "\003VO\001", 4); // SET_CAMERA_VIDEO_OUTPUT to herobus
//            SendBufToCamera();
          }
#endif
          // td lets camera into 3D mode
          // camera will send HBFF
          queueIn("td");
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
            queueIn("VO1"); // SET_CAMERA_VIDEO_OUTPUT to herobus
          } else {
#ifdef USE_GENLOCK
            __debug(F("slave bacpac and use genlock"));
            {
              // dummy setting: should be overridden soon
              char tmptd[TD_BUFFER_SIZE] = {
#define MODE_VIDEO 0x00
#define MODE_PHOTO 0x01
#define MODE_BURST 0x02
#define MODE_TIMELAPSE 0x03
#define MODE_DUAL 0x08
              0x28, 'T', 'D', 0x0f, 0x01, 0x12, 0x04, 0x0d, 0x33, MODE_PHOTO,
              0x05, 0xff, 0x03, 0x07, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00,
              0x01, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0a, };
              memcpy(buf, tmptd, TD_BUFFER_SIZE);
              memcpy(td, tmptd, TD_BUFFER_SIZE);
            }
            SendBufToCamera();
#else
            __debug(F("slave bacpac and not use genlock"));
            queueIn("XS1");
#endif
          }
        }
      } else if (recv[0] == 0x27) {
        // Usual packet length (recv[0]) is 0 or 1.
        // Packet length 0x27 does not exist but SMARTY_START
        memcpy((char *)td+1, recv, TD_BUFFER_SIZE-1);
        td[0] = TD_BUFFER_SIZE-1; td[1] = 'T'; td[2] = 'D'; // get ready to submit to slave
        _setTime();
        setupTimeAlarms();
#ifdef USE_GENLOCK
        if (isMaster()) {
          queueIn("FN0C"); // emulate slave ready
          // send camera config to master dongle
          Serial.print("TD");
          for (int i = 3; i < TD_BUFFER_SIZE; i++) {
            char tmp[3];
            sprintf(tmp, "%02X", td[i]);
            Serial.print(tmp);
          }
          Serial.println("");
          Serial.flush();
        }
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

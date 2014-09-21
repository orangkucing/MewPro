boolean powerOnAtCameraMode = false;

const int GET_BACPAC_PROTOCOL_VERSION = 'v'*256 + 's';
const int SET_BACPAC_SHUTTER_ACTION   = 'S'*256 + 'H';
const int SET_BACPAC_3D_SYNC_READY    = 'S'*256 + 'R';
const int SET_BACPAC_WIFI             = 'W'*256 + 'I'; // Defunct
const int SET_BACPAC_FAULT            = 'F'*256 + 'N';
const int SET_BACPAC_POWER_DOWN       = 'P'*256 + 'W';
const int SET_BACPAC_SLAVE_SETTINGS   = 'X'*256 + 'S';
const int SET_BACPAC_HEARTBEAT        = 'H'*256 + 'B';

void bacpacCommand()
{
  switch (recv[1]*256 + recv[2]) {
  case GET_BACPAC_PROTOCOL_VERSION:
    ledOff();
    buf[0] = 1; buf[1] = 1; // OK
    SendBufToCamera();
    delay(1000); // need some delay before I2C EEPROM read
    if (isMaster()) {
      queueIn("VO1"); // SET_CAMERA_VIDEO_OUTPUT to herobus
    } else {
      queueIn("XS1");
    }
#ifdef USE_TIME_ALARMS
    if (timeStatus() != timeSet) {
      Serial.println("time not set");
      queueIn("td"); // Synchronize time with camera.
    } else {
      char buf[64];
      Serial.println("time already set");
      sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
      Serial.println(buf);
      setupTimeAlarms();
    }
#endif
    break;
  case SET_BACPAC_3D_SYNC_READY:
    switch (recv[3]) {
    case 0: // CAPTURE_STOP
      // video stops at FALLING edge in MASTER NORMAL mode
      digitalWrite(TRIG, HIGH);
      delayMicroseconds(3);
      digitalWrite(TRIG, LOW);
      break;
    case 1: // CAPTURE_START
      if (powerOnAtCameraMode) {
        ledOff();
      }
      break;
    default:
      break;
    }
    break;
  case SET_BACPAC_SLAVE_SETTINGS:
    if (recv[9] == 0 && recv[10] == 0) {
      powerOnAtCameraMode = true;
    }
    // every second message will be off if we send "XS0" here
    queueIn("XS0");
    // battery level: 0-3 (4 if charging)
    Serial.print(" batt_level:"); Serial.print(recv[4]);
    // photos remaining
    Serial.print(" remaining:"); Serial.print(recv[5] * 256 + recv[6]);
    // photos on microSD card
    Serial.print(" photos:"); Serial.print(recv[7] * 256 + recv[8]);
    // video time remaining (sec)
    Serial.print(" seconds:"); Serial.print(recv[9] * 256 + recv[10]);
    // videos on microSD card
    Serial.print(" videos:"); Serial.print(recv[11] * 256 + recv[12]);
    {
      // maximum file size (4GB if FAT32, 0 means infinity if exFAT)
      // if one video file exceeds the limit then GoPro will divide it into smaller files automatically
      char tmp[13];
      sprintf(tmp, " %02xGB %02x%02x%02x", recv[13], recv[14], recv[15], recv[16]);
      Serial.print(tmp);
    }
    Serial.println();
    break;
  case SET_BACPAC_HEARTBEAT: // response to GET_CAMERA_SETTING
    // to exit 3D mode, emulate detach bacpac
    pinMode(BPRDY, INPUT);
    delay(1000);
    pinMode(BPRDY, OUTPUT);
    digitalWrite(BPRDY, LOW);
    break;
  default:
    break;
  }
}

void checkBacpacCommands()
{
  if (recvq) {
    _printInput();
    if (!(recv[0] & 0x80)) {// information bytes
      switch (recv[0]) {
        // Usual packet length (recv[0]) is 0 or 1.
        case 0x27: // Packet length 0x27 does not exist but SMARTY_START
          // Information on received packet here.
          //
          // recv[] meaning and/or relating bacpac command
          // -----+-----------------------------------------------------
          // 0x00   packet length (0x27)
          // 0x01   always 0
          //        TM SET_BACPAC_DATE_TIME
          // 0x02     year (0-99)    
          // 0x03     month (1-12)
          // 0x04     day (1-31)
          // 0x05     hour (0-23)
          // 0x06     minute (0-59)
          // 0x07     second (0-59)
          // 0x08   CM SET_BACPAC_MODE
          // 0x09   PR SET_BACPAC_PHOTO_RESOLUTION
          // 0x0a   VR SET_BACPAC_VIDEORESOLUTION (Defunct; always 0xff)
          // 0x0b   VV SET_BACPAC_VIDEORESOLUTION_VV
          // 0x0c   FS SET_BACPAC_FRAMES_PER_SEC
          // 0x0d   FV SET_BACPAC_FOV
          // 0x0e   EX SET_BACPAC_EXPOSURE
          // 0x0f   TI SET_BACPAC_PHOTO_XSEC
          // 0x10   TS SET_BACPAC_TIME_LAPSE (Defunct; always 0xff)
          // 0x11   BS SET_BACPAC_BEEP_SOUND
          // 0x12   VM SET_BACPAC_NTSC_PAL
          // 0x13   DS SET_BACPAC_ONSCREEN_DISPLAY
          // 0x14   LB SET_BACPAC_LEDBLINK
          // 0x15   PN SET_BACPAC_PHOTO_INVIDEO
          // 0x16   LO SET_BACPAC_LOOPING_MODE
          // 0x17   CS SET_BACPAC_CONTINUOUS_SHOT
          // 0x18   BU SET_BACPAC_BURST_RATE
          // 0x19   PT SET_BACPAC_PROTUNE_MODE
          // 0x1a   AO SET_BACPAC_AUTO_POWER_OFF
          // 0x1b   WB SET_BACPAC_WHITE_BALANCE
          // 0x1c   (reserved)
          // 0x1d   (reserved)
          // 0x1e   (reserved)
          // 0x1f   (reserved)
          // 0x20   (reserved)
          // 0x21   (reserved)
          // 0x22   UP SET_BACPAC_FLIP_MIRROR
          // 0x23   DM SET_BACPAC_DEFAULT_MODE
          // 0x24   CO SET_BACPAC_PROTUNE_COLOR
          // 0x25   GA SET_BACPAC_PROTUNE_GAIN
          // 0x26   SP SET_BACPAC_PROTUNE_SHARPNESS
          // 0x27   EV SET_BACPAC_PROTUNE_EXPOSURE_VALUE
          // -----+-----------------------------------------------------
#ifdef USE_TIME_ALARMS
          // Hour, Min, Sec, date, month, full 4-digit year
          setTime(recv[5], recv[6], recv[7], recv[4], recv[3], 2000 + recv[2]);
          setupTimeAlarms();
#endif
          break;
        default:
          // do nothing
          break;
      }
    } else { 
      bacpacCommand();
    }
    recvq = false;
  }
}

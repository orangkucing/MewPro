#if !defined(__MK20DX256__) && !defined(__MK20DX128__) // not Teensy 3.x
#define I2C_NOSTOP false
#define I2C_STOP true
#endif

#if defined (__SAM3X8E__) // Arduino Due only
// GoPro camera already has pull-up resistors on the I2C bus inside. 
// Due's Wire lib, however, uses D20 and D21 as SDA and SCL respectively, which have pull-up resistors of 1k ohm, too. 
// Thus in order to avoid the conflict of resistors we must use non pull-up'ed D70 and D71 as SDA and SCL respectively,
// and these correspond to Wire1 lib here.
#define WIRE              Wire1
#else
// standard Wire library enables ATmega internal pull-ups by default. usually the resistors are 20k ohm or greater thus
// these should cause no harm. 
#define WIRE              Wire
#endif

// GoPro Dual Hero EEPROM IDs
const int ID_MASTER = 4;
const int ID_SLAVE  = 5;

// I2C slave addresses
const int I2CEEPROM = 0x50;
const int SMARTY = 0x60;

// Camera accesses I2C EEPROM located at slave address 0x50 using 8-bit word address.
// So any one of 3.3V EEPROMs 24XX00, 24XX01, 24XX02, 24XX04, 24XX08, or 24XX16 (XX = AA or LC) works with camera.
//   Note: If not pin-compatible 24AA00 (SOT-23) is used, PCB modifications as well as following value changes are nesessary:
//     const int WRITECYCLETIME = 4000;
//     const int PAGESIZE = 1;
//
// cycle time in milliseconds after block write
const int WRITECYCLETIME = 5000;
//
// page size for block write
const int PAGESIZE = 8; // 24XX01, 24XX02
// const int PAGESIZE = 16; // 24XX04, 24XX08, 24XX16

byte buf[MEWPRO_BUFFER_LENGTH], recv[MEWPRO_BUFFER_LENGTH];

int bufp = 1;
volatile boolean recvq = false;

// interrupt
#if defined(__MK20DX256__) || defined(__MK20DX128__)
void receiveHandler(size_t numBytes)
#else
void receiveHandler(int numBytes)
#endif
{
  int i = 0;
  while (WIRE.available()) {
    recv[i++] = WIRE.read();
    recvq = true;
  }
  if (i == 4 && strncmp((char *)recv, "\203SR", 3) == 0) {
    if (recv[3] == 1) {
      ledOn();
    } else {
      ledOff();
    }
  }
}

void requestHandler()
{
  digitalWrite(I2CINT, HIGH);
  WIRE.write(buf, (int) buf[0] + 1);
}

// print out debug information to Arduino serial console
void __debug(const __FlashStringHelper *p)
{
  if (debug) {
    Serial.println(p);
  }
}

void __printBuf(byte *p)
{
  int buflen = p[0] & 0x7f;

  for (int i = 0; i <= buflen; i++) {
    if (i == 1 && isprint(p[1]) || i == 2 && p[1] != 0 && isprint(p[2])) {
      if (i == 1) {
        Serial.print(' ');
      }
      Serial.print((char) p[i]);
    } else {
      char tmp[4];
      sprintf(tmp, " %02x", p[i]);
      Serial.print(tmp);
    }
  }
  Serial.println("");
  Serial.flush();
}

void _printInput()
{
  if (debug) {
    Serial.print('>');
    __printBuf(recv);
  }
}

void SendBufToCamera() {
  if (debug) {
    Serial.print('<');
    __printBuf(buf);
  }
  digitalWrite(I2CINT, LOW);
}

void resetI2C()
{
  WIRE.begin(SMARTY);
  WIRE.onReceive(receiveHandler);
  WIRE.onRequest(requestHandler);

  emptyQueue();
}

// Read I2C EEPROM
boolean isMaster()
{
  if (eepromId == 0) {
    WIRE.begin();
    WIRE.beginTransmission(I2CEEPROM);
    WIRE.write((byte) 0);
    WIRE.endTransmission(I2C_NOSTOP);
    WIRE.requestFrom(I2CEEPROM, 1, I2C_STOP);
    if (WIRE.available()) {
      eepromId = WIRE.read();
    }

    resetI2C();
  }
  return (eepromId == ID_MASTER);
}

// SET_CAMERA_3D_SYNCHRONIZE START_RECORD
void startRecording()
{
  queueIn("SY1");
}

// SET_CAMERA_3D_SYNCHRONIZE STOP_RECORD
void stopRecording()
{
  queueIn("SY0");
}

// Camera power On
void powerOn()
{
  pinMode(PWRBTN, OUTPUT);
  digitalWrite(PWRBTN, LOW);
  delay(1000);
//  tdDone = false;
  pinMode(PWRBTN, INPUT);
}

// Write I2C EEPROM
void roleChange()
{
  byte id, d;
  // emulate detouching bacpac by releasing BPRDY line
  pinMode(BPRDY, INPUT);
  delay(1000);

  id = isMaster() ? ID_SLAVE : ID_MASTER;
  
  WIRE.begin();
  for (unsigned int a = 0; a < 16; a += PAGESIZE) {
    WIRE.beginTransmission(I2CEEPROM);
    WIRE.write((byte) a);
    for (int i = 0; i < PAGESIZE; i++) {
      switch ((a + i) % 4) {
        case 0: d = id; break; // major (MOD1): 4 for master, 5 for slave
        case 1: d = 5; break;  // minor (MOD2) need to be greater than 4
        case 2: d = 1; break;
        case 3: d = (id == 4 ? 0x0a : 0x0b); break;
      }
      WIRE.write(d);
    }
    WIRE.endTransmission(I2C_STOP);
    delayMicroseconds(WRITECYCLETIME);
  }
  pinMode(BPRDY, OUTPUT);
  eepromId = id;
  digitalWrite(BPRDY, LOW);
  resetI2C();
}

void checkCameraCommands()
{
  while (inputAvailable())  {
    static boolean shiftable;
    byte c = myRead();
    switch (c) {
      case ' ':
        continue;
      case '\r':
      case '\n':
        if (bufp != 1) {
          buf[0] = bufp - 1;
          bufp = 1;
          SendBufToCamera();
        }
        return;
      case '&':
        bufp = 1;
        debug = !debug;
        __debug(F("debug messages on"));
        while (inputAvailable()) {
          if (myRead() == '\n') {
            return;
          }
        }
        return;  
      case '@':
        bufp = 1;
        __debug(F("camera power on"));
        powerOn();
        while (inputAvailable()) {
          if (myRead() == '\n') {
            return;
          }
        }
        return;
      case '!':
        bufp = 1;
        __debug(F("role change"));
        roleChange();
        while (inputAvailable()) {
          if (myRead() == '\n') {
            return;
          }
        }
        return;
      default:
        if (bufp >= 3 && isxdigit(c)) {
          c -= '0';
          if (c >= 10) {
            c = (c & 0x0f) + 9;
          }    
        }
        if (bufp < 4) {
          shiftable = true;
          buf[bufp++] = c;
        } else {
          if (shiftable) { // TM requires six args; "TM0e080a0b2d03" sets time to 2014 Aug 10 11:45:03
            buf[bufp-1] = (buf[bufp-1] << 4) + c;
          } else {
            buf[bufp++] = c;
          }
          shiftable = !shiftable;      
        }
        break;
    }
  }
}



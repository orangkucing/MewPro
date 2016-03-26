// interface to 7 segment LED connected through 74HC595
//
// a wired status monitor for GoPro cameras:
//    -                             power off
//    4 - 0                         battery status (4: charging, 3: full, ..., 0: empty)
//    C                             no sd Card
//    blink and decimal point on    recording/shooting
//    fast blink 8                  sd is almost full
//
#ifdef USE_TURNED_ON

// in order to use Turned On functionalities with MewPro Cable we assign 74HC595 lines to MewPro's TxO/RxI and SHUTTER_PIN.
//
// the following setting will conflict with UART and USE_SHUTTER
//                                 Arduino Pro Mini      ATtiny1634
const int turnedOn_data = 1; //     TxO                  RxI
const int turnedOn_clk = 0;  //     RxI                  TxO
const int turnedOn_latch = SHUTTER_PIN;
// if you want to use both Turned On and UART then comment out/uncomment the previous/following two lines: 
//const int turnedOn_data = 3;
//const int turnedOn_clk = 4;
// if you want to USE_SHUTTER then change the following line.
//const int turnedOn_latch = SHUTTER_PIN;

const byte digits[] = {
  0b11111100, // 0 battery status
  0b00001100, // 1 battery status
  0b11011010, // 2 battery status
  0b11110010, // 3 battery status
  0b01100110, // 4 battery status (charging)
  0b10110110, // 5
  0b00111110, // 6
  0b11100000, // 7
  0b11111110, // 8 initializing LED or warning: sd is almost full
  0b11100110, // 9 (don't use. use '6' only)
  0b11101110, // A
  0b00111110, // b (don't use. use '6' only)
  0b10011100, // C error: no sd Card
  0b01111010, // d
  0b10011110, // E (don't use. use '3' only)
  0b10001110, // F
  0b00000010, // - off
};

boolean turnedOn, dpState;
unsigned long lastUpdateStatus;
uint8_t batt_level;
uint16_t remaining;
uint16_t oldremaining;
uint16_t oldvideos;
uint16_t oldphotos;
int oldstatus;


void _initStatus()
{
  turnedOn = false;
  dpState = false;
  oldremaining = 0xffff;
  oldvideos = 0xffff;
  oldphotos = 0xffff;
  oldstatus = -1;
}

void showStatus()
{
  int newstatus;
  
  if (remaining == 0xffff) {
    newstatus = digits[0x0C]; // error : no sD
  } else {
    newstatus = digits[batt_level];
  }
  newstatus = turnedOn ? (newstatus | (dpState ? 1 : 0)) : digits[0x10];
  if (oldstatus != newstatus) {
    digitalWrite(turnedOn_latch, LOW);
    shiftOut(turnedOn_data, turnedOn_clk, LSBFIRST, newstatus);
    digitalWrite(turnedOn_latch, HIGH);
  }
  oldstatus = newstatus;
}

void updateStatus()
{
  static char count;

  batt_level = RECV(4);
  remaining = (RECV(5) << 8) + RECV(6);
  uint16_t photos = (RECV(7) << 8) + RECV(8);
  uint16_t minutes = (RECV(9) << 8) + RECV(10);
  uint16_t videos = (RECV(11) << 8) + RECV(12);

  turnedOn = true;  
  if (oldremaining == 0xffff || remaining == 0xffff) {
    oldremaining = remaining; oldvideos = videos; oldphotos = photos;
    dpState = false;
    showStatus();
    lastUpdateStatus = millis();
    return;
  }
  // sd is almost Full
  if (remaining < 5 || minutes == 1) {
    oldstatus = digits[0x08] | (dpState ? 1 : 0);
    digitalWrite(turnedOn_latch, LOW);
    shiftOut(turnedOn_data, turnedOn_clk, LSBFIRST, oldstatus);
    digitalWrite(turnedOn_latch, HIGH);
    delay(100);
  }
  if (dpState) {
    oldstatus = 1;
    digitalWrite(turnedOn_latch, LOW);
    shiftOut(turnedOn_data, turnedOn_clk, LSBFIRST, oldstatus);
    digitalWrite(turnedOn_latch, HIGH);
    delay(200);
  }
  if (oldphotos < photos || oldvideos < videos || oldremaining > remaining) {
    // recording has been started by using camera's shutter button
    count = 3;
    dpState = true;
  } else if (oldremaining == remaining) {
    count--;
    if (count == 0) {
      dpState = false; // stop recording
    }
  }
  showStatus();
  lastUpdateStatus = millis();
  oldremaining = remaining; oldvideos = videos; oldphotos = photos;
}

void checkStatus()
{
  unsigned long time = millis();
  if (time - lastUpdateStatus > 2000) {
    lastUpdateStatus = time;
    _initStatus();
    showStatus();
  }
}

void setupTurnedOn()
{
  pinMode(turnedOn_latch, OUTPUT);
  pinMode(turnedOn_clk, OUTPUT);
  pinMode(turnedOn_data, OUTPUT);
  lastUpdateStatus = millis();
  _initStatus();
  showStatus();
}

#else // undef USE_TURNED_ON

void updateStatus()
{
}

void checkStatus()
{
}

void setupTurnedOn()
{
}

#endif // USE_TURNED_ON

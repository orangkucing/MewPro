// Video Motion Detector using intersil EL1883 Sync Separator
//
// 2014 (c) orangkucing
//
#ifdef USE_VIDEOMOTION

// This part of the sketch exclusively uses the following pins. 
// (If an other part uses one of these pins please #undef that part.)
//
// if Teensy 3.1 or Teensy 3.0
//   D2 (INT2) : Vertical Sync = pin 3 of EL1883.
//   D3 (INT3) : Horizontal Sync = pin 7 of EL1883.
//   A9/D23 (CMP1_IN0) : Analog comparator negative input. (voltage to compare)
// if GR-KURUMI (RL78/G13 has no built-in analog comparator)
//     * Remark: Renesas RL78/G13 has 10 external interrupts (INTP0-INTP6, INTP8, INTP9, INTKR)
//     * but GR-KURUMI made all of these pins unusable except
//     *   INTP3 on D2 (as Arduino compatible interrupt number 0)
//     *   INTP5 on D3 (as Arduino compatible interrupt number 1)
//     *   INTP4 on D4 (not used in standard library).
//     * Since this code needs three interrupt pins, we assign as
//     *   D2 : Vertical Sync
//     *   D3 : Horizontal Sync
//     * and additional
//     *   D4 : Input from external analog comparator.
//     * In order to use D4 as an interrupt pin we need to do it low level and modifications on
//     * libraries are necessary. (sigh..)
//     * "How To Do It" documents about GR-KURUMI is in preparation...
//   D2 (INT0) : Vertical Sync = pin 3 of EL1883.
//   D3 (INT1) : Horizontal Sync = pin 7 of EL1883.
//   D4 (INT2) : Input from external analog comparator
//   D9 (PWM) : Constant reference PWM voltage for brightness threshold.
// if Arduino Pro Mini
//   D2 (INT0) : Vertical Sync = pin 3 of EL1883.
//   D3 (INT1) : Horizontal Sync = pin 7 of EL1883.
//   D6 (AIN0) : Analog comparator positive input. (fixed voltage)
//   D7 (AIN1) : Analog comparator negative input. (voltage to compare)
//   D9 (OC1A) : Constant reference PWM voltage for brightness threshold.
// if Arduino Pro Micro
//   D0 (INT2) : Vertical Sync = pin 3 of EL1883.
//   D1 (INT3) : Horizontal Sync = pin 7 of EL1883.
//   D7 (AIN0) : Analog comparator positive input. (fixed voltage)
//     * Note: Arduino Pro Micro has no AIN1 pin.
//   D8 (A8)   : Analog comparator negative input. (voltage to compare)
//   D9 (OC1A) : Constant reference PWM voltage for brightness threshold.


// Bulk motion detection:
// volatile variables

volatile uint16_t interlace;
volatile uint16_t currentLine;
volatile uint16_t lastHsyncTime;
volatile boolean nosignal = true;

// Composite video signal:
//   White level = 1.0V (77/255 or 19/63)
//   Black level = 0.3V (23/255 or 5/63)

//****************************************************************
//  Teensy 3.1             || Teensy 3.0             || Teensy LC
#if defined(__MK20DX256__) || defined(__MK20DX128__) || defined(__MKL26Z64__) 
const int SCANLINE_OFFSET = 94;
const int MAX_SCANLINES = 100;
typedef uint32_t image_t; // 1 word = 32 bits
#define SIZE_OF_IMAGE_T 4
const int WORD_PER_LINE = 2; // 2^2 words = 128 bits
// set brightnessThreshold between 5: very sensitive and 19: not sensitive.
const int brightnessThreshold = 10; // CHANGE ME!!
const image_t differenceThreshold = 2000L; // CHANGE ME!!
#define VSYNC_PIN 2
#define HSYNC_PIN 3

//****************************************************************
//    GR-KURUMI
#elif defined(REL_GR_KURUMI)
const int SCANLINE_OFFSET = 94;
const int MAX_SCANLINES = 100;
typedef uint32_t image_t; // 1 word = 32 bits
#define SIZE_OF_IMAGE_T 4
const int WORD_PER_LINE = 2; // 2^2 words = 128 bits
// set brightnessThreshold between 23: very sensitive and 77: not sensitive.
const int brightnessThreshold = 50; // CHANGE ME!!
const image_t differenceThreshold = 1000L; // CHANGE ME!!
#define VSYNC_PIN 2
#define HSYNC_PIN 3
// GR-KURUMI IDE doesn't define the following useful macro...
#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(p) (p - 2)
#endif

//****************************************************************
//    Arduino Pro Mini            || Arduino Pro Micro
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)
//
#if F_CPU == 16000000
// overclocked Arduino Pro Mini/Micro 3.3V
// GoPro needs 3.3V logic. Running 16MHz on ATmega 328P/32U4 is not recommended at 3.3V.
// (Note: ATmega 328P can run 20MHz but it is not supported by Arduino IDE)
const int SCANLINE_OFFSET = 110;
const int MAX_SCANLINES = 70;
typedef uint16_t image_t; // 1 word = 16 bits
#define SIZE_OF_IMAGE_T 2
const int WORD_PER_LINE = 1; // 2^1 words = 32 bits
// normal Arduino Pro Mini/Micro 3.3V
#elif F_CPU == 8000000
const int SCANLINE_OFFSET = 94;
const int MAX_SCANLINES = 100;
typedef uint16_t image_t; // 1 word = 16 bits
#define SIZE_OF_IMAGE_T 2
const int WORD_PER_LINE = 0; // 2^0 words = 16 bits
#endif
//
// set brightnessThreshold between 23: very sensitive and 77: not sensitive.
const int brightnessThreshold = 50; // CHANGE ME!!
const image_t differenceThreshold = 300; // CHANGE ME!!
#if defined(__AVR_ATmega328P__)
#define VSYNC_PIN 2
#define HSYNC_PIN 3
#elif defined(__AVR_ATmega32U4__)
#define VSYNC_PIN 0
#define HSYNC_PIN 1
#endif

//****************************************************************
#else
#error CPU not supported
#endif

volatile image_t diff, tmpdiff;
// buffer for scanning lines located at the center part.
// effective scanning lines in NTSC (PAL) = 240 * 2 (288 * 2).
volatile image_t line[1 << WORD_PER_LINE];
volatile image_t image[(MAX_SCANLINES << 1) * (1 << WORD_PER_LINE)];
volatile image_t imagediff[MAX_SCANLINES * (1 << WORD_PER_LINE)];

int VMDstatus = 0;
unsigned long epoch;

void motionEnd()
{
  VMDstatus = 4;
  __debug(F("stop"));
  stopRecording();
  epoch = millis();
}

void motionDetected()
{
  if (!ledState) {
    VMDstatus = 3;
    __debug(F("start"));
    startRecording();
  }
  epoch = millis();
}

// interrupt routines

void VSyncHandler()
{
  uint16_t i;
  image_t d;
	
  currentLine = 3; // VSync always occurs in line 4 (or 267) (base 1).
  
  // For detailed info on HSync and VSync timing, this is very good document:
  //   "MAX9568 Component Analog TV Sync Separator datasheet"

#if defined(REL_GR_KURUMI) // GR-KURUMI's micros() is too slow so we must directly read decrementing timer counter
  if (lastHsyncTime - TCR05.tcr05 < 32) {
#else
  if ((uint16_t)micros() - lastHsyncTime < 32) {
#endif
    // beginning of an even frame
    interlace = 1;
    nosignal = false;
  } else {
    // beginning of an odd frame
    interlace = 0;
  }
  for (i = 0; i < MAX_SCANLINES * (1 << WORD_PER_LINE); i++) {
    d = imagediff[i];
    // nasty but fast bitcount
#if SIZE_OF_IMAGE_T == 4
    d = d - ((d >> 1) & 0x55555555);
    d = (d & 0x33333333) + ((d >> 2) & 0x33333333);
    d = (d + (d >> 4)) & 0x0f0f0f0f;
    d = (d + (d >> 8)) & 0x00ff00ff;
    tmpdiff += (d + (d >> 16)) & 0x0000ffff;
#elif SIZE_OF_IMAGE_T == 2
    d = d - ((d >> 1) & 0x5555);
    d = (d & 0x3333) + ((d >> 2) & 0x3333);
    d = (d + (d >> 4)) & 0x0f0f;
    tmpdiff += (d + (d >> 8)) & 0x00ff;
#else // SIZE_OF_IMAGE_T == 1
    d = d - ((d >> 1) & 0x55);
    d = (d & 0x33) + ((d >> 2) & 0x33);
    tmpdiff += (d + (d >> 4)) & 0x0f;
#endif
  }
  if (interlace == 0) {
    diff = tmpdiff;
    tmpdiff = 0;
  }
}

void HSyncHandler()
{
  uint16_t i, c, d;

#if defined(REL_GR_KURUMI) // workaround for GR-KURUMI's very slow micros()
  lastHsyncTime = TCR05.tcr05;
#else
  lastHsyncTime = (uint16_t)micros();
#endif
  c = currentLine++ - SCANLINE_OFFSET;
  d = c;
  
  if (c < MAX_SCANLINES) {
    if (interlace) {
      c += MAX_SCANLINES;
    }
    c <<= WORD_PER_LINE; d <<= WORD_PER_LINE;
    for (i = 0; i < (1 << WORD_PER_LINE); i++) {
      imagediff[d + i] = image[c + i] ^ line[i];
      image[c + i] = line[i];
    }
  }
  memset((void *)line, 0, sizeof(line));
}

#if defined(__MK20DX256__) || defined(__MK20DX128__) || defined(__MKL26Z64__) // Teensy 3.x/LC

void changeHandler()
{
  uint16_t x;
 
  x = (uint16_t)micros() - lastHsyncTime;

  // HSync period is 63.56μs and sync-to-blanking-end 9.48μs
  if (x <= 64 && x >= 10) {
    x -= 10;
    if ((CMP1_SCR & 1)) { // check CMP_SCR_COUT and store 2 bit per interrupt
      // INP > INM
      line[x >> 4] |= (image_t)1 << ((x & B1111) << 1);
    } else {
      // INP < INM
      line[x >> 4] |= (image_t)1 << (((x & B1111) << 1) | 1);
    }
  }
}

void _resetCMP()
{
  // prepare capturing video frames.
  // we use 6bit DAC1 for INP.
  analogComparator1.setOn(7, 0); // set comparator input PSEL=IN7 (6b DAC1 Reference) / MSEL=IN0 (D23)
  // set the reference analog voltage.
  CMP1_DACCR = B11000000 | brightnessThreshold; // enable 6b DAC1 and use VDD as reference
  analogComparator1.enableInterrupt(changeHandler, CHANGE);
}

#elif defined(REL_GR_KURUMI) // GR-KURUMI

void changeHandler()
{
  uint16_t x;

  x = lastHsyncTime - (uint16_t)TCR05.tcr05; // TCR05 counts down every micro seconds

  // HSync period is 63.56μs and sync-to-blanking-end 9.48μs
  if (x <= 64 && x >= 10) {
    x -= 10;
    // D4 = P31 (port 3, bit 1)
    if ((*((volatile uint8_t*)ADDR_PORT_REG + PORT_3) & B00000010)) { // store 2 bit per interrupt
      // INP > INM
      line[x >> 4] |= (image_t)1 << ((x & B1111) << 1);
    } else {
      // INP < INM
      line[x >> 4] |= (image_t)1 << (((x & B1111) << 1) | 1);
    }
  }
}

void _resetCMP()
{
  // Prepare capturing video frames:
  //   D9 is used as the analog threshold voltage on external analog comparator.
  pinMode(9, OUTPUT);
  //   Comparator's positive reference pin is connected to D9 via a low-pass RC filter
  //   (R = 3.9k and C = 0.1μF). In order to keep the voltage stable, PWM frequency on D9 pin
  //   must be as high as possible: 
  //       cf. gr_common/RLduino78/cores/RLduino78_basic.cpp

  analogWriteFrequency(32000L);
  analogWrite(9, brightnessThreshold);
  
  pinMode(4, INPUT_PULLUP);
  // in order to attach interrupt #2 on D4, modification on RLduino78 files are necessary.
  // (not well documented yet)
  attachInterrupt(digitalPinToInterrupt(4), changeHandler, CHANGE);
}

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) // Arduino Pro Mini or Arduino Pro Micro

void changeHandler()
{
  uint16_t x;

  x = (uint16_t)micros() - lastHsyncTime;

  // HSync period is 63.56μs.
  // subtract the amount for sync-to-blanking-end 9.48μs.
  x -= 10;

  // on 8MHz/16MHz Arduinos micros() has a resolution of 8μs/4μs respectively.
  // actual brightness information Arduino Pro Mini 8MHz can capture from each scan line is only 5 (or 6) samples. 
  x >>= 2;
#if F_CPU == 16000000 // overclocked Arduino Pro Mini
  // store two bit per interrupt.
  if ((ACSR & (1 << ACO))) {
    // AIN+ > AIN-
    line[x >> 3] |= (image_t)1 << ((x & B111) << 1);
  } else {
    // AIN+ > AIN-
    line[x >> 3] |= (image_t)1 << (((x & B111) << 1) | 1);
  }
#else // F_CPU == 8000000
  // store two bit per interrupt.
  if ((ACSR & (1 << ACO))) {
    // AIN+ > AIN-
    line[0] |= (1 << x);      // store B01
  } else {
    // AIN+ < AIN-
    line[0] |= (1 << (x | 1));    // store B10
  }
#endif
}

void _resetCMP()
{
  // Prepare capturing video frames:
  //   D9 is used as the analog threshold voltage on Arduino analog comparator.
  //   Pro Mini Note: don't change pin D9 to D5 or D6. because PWM on D5 or D6 depends on Timer 0 so as micros() or delay().
  pinMode(9, OUTPUT);
  //   Comparator's positive reference pin AIN0 is connected to D9 via a low-pass RC filter
  //   (R = 3.9k and C = 0.1μF). In order to keep the voltage stable, PWM frequency on D9 pin
  //   must be as high as possible: Set divisor = 1.
  //       cf. http://playground.arduino.cc/Main/TimerPWMCheatsheet
  TCCR1B = TCCR1B & B11111000 | 0x01;
  analogWrite(9, brightnessThreshold);

#if defined(__AVR_ATmega328P__) // Arduino Pro Mini  
  analogComparator.setOn(AIN0, AIN1);
#elif defined(__AVR_ATmega32U4__) // Arduino Pro Micro
  analogComparator.setOn(AIN0, A8);
#endif
  analogComparator.enableInterrupt(changeHandler, CHANGE);
}

#endif

void _resetVMD()
{
  interlace = 0;
  nosignal = true;
  
  pinMode(VSYNC_PIN, INPUT);
  pinMode(HSYNC_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(VSYNC_PIN), VSyncHandler, FALLING); 
  attachInterrupt(digitalPinToInterrupt(HSYNC_PIN), HSyncHandler, FALLING);

  _resetCMP(); // reset analog comparator

  VMDstatus = 2;
  __debug(F("VMD start"));
}

void resetVMD()
{
  __debug(F("VMD will start after 5 seconds."));
  VMDstatus = 1;
  epoch = millis();
}

void checkVMD()
{
  unsigned long n = millis();
  switch (VMDstatus) {
    case 1:
      if (n - epoch > 4500L) { // wait 4.5 sec until starting VMD.
        _resetVMD();
        delay(500); // ignore transition from blank signal to captured signal
      }
      return;
    case 2: // VMD working & no motion detected.
      break;
    case 3: // motion has been detected recently, do nothing for 5 sec.
      if (n - epoch > 5000L) { 
        motionEnd();
        return;
      }
      break;
    case 4: // camera stop command has been sent recently, do nothing for 5 sec.
      if (n - epoch > 5000L) {
        VMDstatus = 2;
      }
      return;
    default:
      break;
  }
  
  if (nosignal) {
    return;
  }

#if 0 // debug image
  for (int c = 0; c < MAX_SCANLINES; c++) {
    for (int i = 0; i < (1 << WORD_PER_LINE); i++) {
      for (int k = 0; k < SIZE_OF_IMAGE_T << 3; k++) {
        Serial_print((image[(c << WORD_PER_LINE) + i] & (1 << k) ? 1 : 0) + (image[(c + MAX_SCANLINES << WORD_PER_LINE) + i] & (1 << k) ? 1 : 0));
      }
    }
    Serial_println("");
  }
  Serial_println((uint16_t)diff);
  Serial_println(F("--------"));
#else
  if (diff > differenceThreshold) { // Change Me!
//  if (debug) {
//      Serial_println((uint16_t)diff);
//  }
    __debug(F("move"));
    motionDetected();
  }
#endif
}

#else

void resetVMD()
{
}

void checkVMD()
{
}

#endif

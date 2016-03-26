// if __AVR_ATmega32U4__ (Leonardo or Pro Micro) then use Serial1 (TTL) instead of Serial (USB) to communicate with genlock dongle
#if defined(USE_GENLOCK) && defined(__AVR_ATmega32U4__)
#define Serial Serial1
#endif

#ifdef USE_TURNED_ON

// UART related functions must be replaced by dummy functions.
void Serial_begin(...) { Serial.end(); }
int Serial_available(...) { return 0; }
int Serial_read(...) { return -1; }
void Serial_print(...) { }
void Serial_println(...) { }
void Serial_flush(...) { }

#else

// use UART
#define Serial_begin Serial.begin
#define Serial_available Serial.available
#define Serial_read Serial.read
#define Serial_print Serial.print
#define Serial_println Serial.println
#define Serial_flush Serial.flush

#endif

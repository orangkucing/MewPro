#define MEWPRO_BUFFER_LENGTH 64

byte queue[MEWPRO_BUFFER_LENGTH];
volatile int queueb = 0, queuee = 0;
boolean waiting = false; // don't read the next command from the queue
boolean serialfirst = false;

byte buf[MEWPRO_BUFFER_LENGTH], recv[MEWPRO_BUFFER_LENGTH];

int bufp = 1;
volatile int recvb = 0, recve = 0;
#define RECV(a) (recv[(recvb + (a)) % MEWPRO_BUFFER_LENGTH])

boolean inputAvailable()
{
  if (!waiting && (queueb != queuee || Serial.available())) {
    return true;
  }
  return false;
}

byte myRead()
{
  if (serialfirst && Serial.available()) {
    return Serial.read();
  }
  if (queueb != queuee) {
    byte c = queue[queueb];
    queueb = (queueb + 1) % MEWPRO_BUFFER_LENGTH;
    serialfirst = false;
    return c;
  }
  serialfirst = true;
  return Serial.read();
}

// Utility functions
void queueIn(const __FlashStringHelper *p)
{
  int i;
  char c;
  for (i = 0; (c = pgm_read_byte((char PROGMEM *)p + i)) != 0; i++) {
    queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = c;
  }
  queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = '\n';
  queuee = (queuee + i + 1) % MEWPRO_BUFFER_LENGTH;
}

void queueIn(const char *p)
{
  int i;
  char c;
  for (i = 0; (c = *(p + i)) != 0; i++) {
    queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = c;
  }
  queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = '\n';
  queuee = (queuee + i + 1) % MEWPRO_BUFFER_LENGTH;
}

void __emptyInputBuffer()
{
  while (inputAvailable()) {
    if (myRead() == '\n') {
      return;
    }
  }
}

void emptyQueue()
{
  queueb = queuee = 0;
  recvb = recve = 0;
  bufp = 1;
  waiting = false;
  serialfirst = false;
  while (Serial.available()) { // clear serial buffer
    Serial.read();
  }
}

// IR remote controller

#ifdef USE_IR_REMOTE

IRrecv irrecv(IRRECV_PIN);
decode_results results;
boolean powerStatus = false;

void IRcommand()
{
  // Example values are from the accessory controller of
  //   DFRobot Digital IR Receiver Module (Arduino), DFR0094
  //   ( http://www.dfrobot.com/index.php?route=product/product&product_id=351 )
  switch (results.value) {
    case 0xFD00FF: // power
      // surprisingly it is difficult to tell the camera is ON or OFF, so we just toggle.
      if (powerStatus) {
        queueIn("PW0"); // SET_CAMERA_POWER_STATE off
      } else {
        powerOn();
      }
      powerStatus = !powerStatus;
      break;
    case 0xFD08F7: // 1 key
      startRecording();
      break;
    case 0xFD30CF: // 0 key
      stopRecording();
      break;
    case 0xFD40BF: // func/stop key
      roleChange(); // role change
      break;
    case 0xFFFFFFFF: // previous key repeated
      break;
    default: // unsupported controller key depressed
      Serial.print(F("IR: "));
      Serial.println(results.value, HEX);
      break;
  }
}

void setupIRremote()
{
  irrecv.enableIRIn();
}

void checkIRremote()
{
  if (irrecv.decode(&results)) {
    IRcommand();
    irrecv.resume(); // Receive the next value
  }
}

#else

void setupIRremote()
{
}

void checkIRremote()
{
}

#endif


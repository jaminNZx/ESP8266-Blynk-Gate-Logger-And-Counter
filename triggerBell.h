/**************************************************************

                     Door Bell Function

 **************************************************************/
void triggerBell() {
  DoorBellButtonCur = digitalRead(PIN_DOORBELL);
  if (DoorBellButtonCur == LOW && DoorBellButtonPrev == HIGH) {
    // BELL PRESSED

    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT
    Blynk.virtualWrite(vPIN_WEBHOOK, 1);

    if (!silentBell) {
      // SEND ALERT TO BASE STATIONS & SMART LED INTERNAL SYSTEM & FRONT LIGHTS
      base.virtualWrite(V1, HIGH);
      leds.virtualWrite(V13, 2);
    }
    if (nightTime) front_lights.virtualWrite(V0, 1);

    // LOG WHEN
    BellLastPressed = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_BELL_LAST, BellLastPressed);

    // TERMINAL
    printOutput("Door Bell Pressed");

    // DAILY COUNTER
    BellDailyCounter++;
    Blynk.virtualWrite(vPIN_BELL_COUNTER, BellDailyCounter);

    // TABLE
    Blynk.virtualWrite(vPIN_BELL_TABLE, "add", tableIndex2, BellLastPressed, 1);
    Blynk.virtualWrite(vPIN_BELL_TABLE, "pick", tableIndex2 );
    tableIndex2++;

    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
  if (DoorBellButtonCur == HIGH && DoorBellButtonPrev == LOW) {
    // perform a secondary function here
    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
}

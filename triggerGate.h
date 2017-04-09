/**************************************************************

                     Gate Logger Function

 **************************************************************/
 void triggerGate() {
  GateSwitchCurrent = digitalRead(PIN_GATE_SWITCH);
  if (GateSwitchCurrent == HIGH && GateSwitchPrev == LOW) {
    GateSwitchFirstTime = millis();
    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT
    Blynk.virtualWrite(vPIN_WEBHOOK, 1);
    // LOG WHEN
    GateLastOpened = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_GATE_LAST, GateLastOpened);
    // INDICATOR COLOUR
    Blynk.setProperty(vPIN_GATE_HELD, "color", "#23C48E");
    
    if (!silentGate) {
      // SEND ALERT TO BASE STATIONS & SMART LED INTERNAL SYSTEM & FRONT LIGHTS
      base.virtualWrite(V1, HIGH);
      leds.virtualWrite(V13, 1);
    }
    if (nightTime) front_lights.virtualWrite(V0, 1);
    
    // DAILY COUNTER
    GateDailyCounter++;
    Blynk.virtualWrite(vPIN_GATE_COUNTER, GateDailyCounter);
    // START TIMER
    timer.enable(timer2);
    // TERMINAL
    printOutput("Gate Opened >>");
  }

  GateSwitchMillisHeld = (millis() - GateSwitchFirstTime);
  GateSwitchSecsHeld = GateSwitchMillisHeld / 1000;

  if (GateSwitchCurrent == LOW && GateSwitchPrev == HIGH) {
    // INDICATOR COLOUR
    Blynk.setProperty(vPIN_GATE_HELD, "color", "#ED9D00");
    // STOP ACTIVE GATE TIMER
    timer.disable(timer2);
    notificationSent = 0;
    // TABLE
    Blynk.virtualWrite(vPIN_GATE_TABLE, "add", tableIndex1, GateLastOpened, formatTime(GateSwitchMillisHeld) );
    Blynk.virtualWrite(vPIN_GATE_TABLE, "pick", tableIndex1 );
    tableIndex1++;
    // TERMINAL
    printOutput(String("Gate Closed << Time: ") + formatTime(GateSwitchMillisHeld));
  }
  GateSwitchPrev = GateSwitchCurrent;
}

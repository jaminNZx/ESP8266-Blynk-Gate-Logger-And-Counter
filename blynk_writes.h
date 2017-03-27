/**************************************************************

                     BLYNK_WRITE

 **************************************************************/
 BLYNK_CONNECTED() {
  base.setAuthToken(AUTH_BASE); // Token for the Base Stations
  leds.setAuthToken(AUTH_LIGHTS); // Token for the Home LED System
  front_lights.setAuthToken(AUTH_FRONT_SWITCH); // Token for the Smart Shelf
}

BLYNK_WRITE(vPIN_GATE_COUNTER) {
  GateDailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_BELL_COUNTER) {
  BellDailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_GATE_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_GATE_TABLE, "clr" );
  printOutput("Front Gate Table Cleared");
}
BLYNK_WRITE(vPIN_BELL_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_BELL_TABLE, "clr" );
  printOutput("Bell Table Cleared");
}

BLYNK_WRITE(vPIN_NOTIFY_DELAY) {
  notifyDelay = param.asInt() * 60;
  if(notifyDelay){
    printOutput(String("Notify Delay: ") + String(param.asInt()) + String(" min"));
  } else {
    printOutput(String("Notify Disabled "));
  }
}

BLYNK_WRITE(vPIN_SILENTMODE) {
  switch (param.asInt()) {
    case 0:
      silentBell = 0;
      silentGate = 0;
      printOutput("Slient Mode Inactive");
      break;
    case 1:
      silentBell = 1;
      silentGate = 0;
      printOutput("Slient Mode: Bell Only");
      break;
    case 2:
      silentBell = 0;
      silentGate = 1;
      printOutput("Slient Mode: Gate Only");
      break;
    case 3:
      silentBell = 1;
      silentGate = 1;
      printOutput("Slient Mode: Bell & Gate");
      break;
  }
}

BLYNK_WRITE(vPIN_NIGHTTIME) {
  nightTime = param.asInt();
  if (nightTime) {
    Blynk.virtualWrite(vPIN_NIGHTTIME_LED, 255);
    printOutput("Night Mode Active");
    timer4 = timer.setTimeout(36000000L, nightTime_END); // 10 HR DELAY TO DISACTIVATE
  } else {
    nightTime = 0;
    Blynk.virtualWrite(vPIN_NIGHTTIME_LED, 0);
    printOutput("Night Mode Inactive (Manual)");
    timer.disable(timer4);
  }
}


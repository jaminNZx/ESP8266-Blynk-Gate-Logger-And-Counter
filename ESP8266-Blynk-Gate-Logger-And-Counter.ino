//#define Blynk_DEBUG
//#define Blynk_PRINT Serial

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "settings.h"
#include <wifi_credentials.h>

WidgetTerminal terminal(vPIN_TERMINAL);
WidgetBridge base(vPIN_BRIDGE_BASE);
WidgetBridge leds(vPIN_BRIDGE_LED);
WidgetBridge front_lights(vPIN_BRIDGE_FRONT_LIGHTS);
WidgetRTC rtc;
SimpleTimer timer;

int           DoorBellButtonCur, GateSwitchCurrent, notificationSent;
byte          DoorBellButtonPrev = HIGH;
long          GateSwitchMillisHeld, GateSwitchSecsHeld, notifyDelay;
byte          GateSwitchPrev = LOW;
unsigned long GateSwitchFirstTime;       // how long since the button was first pressed
String        extraZeroH, extraZeroM, extraZeroS;
int           timer1, timer2, timer3, timer4;
int           GateDailyCounter, BellDailyCounter;
String        GateLastOpened, BellLastPressed;
int           tableIndex1 = 0, tableIndex2 = 0, silentBell, silentGate;
bool          resetCountersDone, nightTime = 0;

String getCurrentTime() {
  extraZeroH = "";
  extraZeroM = "";
  extraZeroS = "";
  if (hour() < 10) {
    extraZeroH = '0';
  }
  if (minute() < 10) {
    extraZeroM = '0';
  }
  if (second() < 10) {
    extraZeroS = '0';
  }
  return String(extraZeroH + hour()) + ':' + extraZeroM + minute() + ':' + extraZeroS + second();
}
String getCurrentDate() {
  return String(day()) + '-' + monthShortStr(month()) + '-' + year();
}

void printOutput(String a) {
  printTimeDate();
  terminal.println(a);
  terminal.flush();
}
void printTimeDate() {
  terminal.println("-----------------------");
  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}
void sendUptime() {
  Blynk.virtualWrite(vPIN_CUR_DATE,  getCurrentDate() + String("  ") + getCurrentTime() );
  Blynk.setProperty(vPIN_CUR_DATE, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
}

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
    if (nightTime) { // ONLY TRIGGER FRONT PATH LIGHTS BETWEEN 9pm-6am
      front_lights.virtualWrite(V0, 1);
    }
    // LOG WHEN
    BellLastPressed = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_BELL_LAST, BellLastPressed);

    // TERMINAL
    printTimeDate();
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

void triggerGate() {
  GateSwitchCurrent = digitalRead(PIN_GATE_SWITCH);
  if (GateSwitchCurrent == HIGH && GateSwitchPrev == LOW) {
    GateSwitchFirstTime = millis();
    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT
    Blynk.virtualWrite(vPIN_WEBHOOK, 1);
    // LOG WHEN
    GateLastOpened = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_GATE_LAST, GateLastOpened);
    // INDICATOR LED
    digitalWrite(PIN_LED, HIGH);
    if (!silentGate) {
      // SEND ALERT TO BASE STATIONS & SMART LED INTERNAL SYSTEM & FRONT LIGHTS
      base.virtualWrite(V1, HIGH);
      leds.virtualWrite(V13, 1);
    }
    if (nightTime) { // ONLY TRIGGER FRONT PATH LIGHTS BETWEEN 9pm-6am
      front_lights.virtualWrite(V0, 1);
    }
    // DAILY COUNTER
    GateDailyCounter++;
    Blynk.virtualWrite(vPIN_GATE_COUNTER, GateDailyCounter);
    // START TIMER
    timer.enable(timer2);
    // TERMINAL
    printOutput("Gate Opened # Timer Started");
  }

  GateSwitchMillisHeld = (millis() - GateSwitchFirstTime);
  GateSwitchSecsHeld = GateSwitchMillisHeld / 1000;

  if (GateSwitchCurrent == LOW && GateSwitchPrev == HIGH) {
    // INDICATOR LED
    digitalWrite(PIN_LED, LOW);
    // STOP ACTIVE GATE TIMER
    timer.disable(timer2);
    notificationSent = 0;
    // TABLE
    Blynk.virtualWrite(vPIN_GATE_TABLE, "add", tableIndex1, GateLastOpened, GateSwitchSecsHeld + String(" sec") );
    Blynk.virtualWrite(vPIN_GATE_TABLE, "pick", tableIndex1 );
    tableIndex1++;
    // TERMINAL
    printOutput(String("Gate Closed # Timer Stopped: ") + GateSwitchSecsHeld + String(" sec"));
  }
  GateSwitchPrev = GateSwitchCurrent;
}

void ActiveGateTimer() {
  Blynk.virtualWrite(vPIN_GATE_HELD, GateSwitchSecsHeld);
  if (notifyDelay && GateSwitchSecsHeld > 0 && GateSwitchSecsHeld > notifyDelay && !notificationSent) {
    Blynk.email("ben@customtabs.co.nz", "Front Gate Alert", GateLastOpened + String(" // Front Gate has been left open for ") + GateSwitchSecsHeld + String(" secs"));
    printOutput(String("Notified # Gate Held: ") + GateSwitchSecsHeld + String(" sec"));
    notificationSent = 1;
    timer3 = timer.setTimeout(((notifyDelay * 1000) - 50), resetNotification);

    //Blynk.sms("Alert! " + GateLastOpened + String(" // Front Gate has been left open for ") + GateSwitchSecsHeld + String(" secs");
    //Blynk.notify("NOTIFY: Alert, Front Gate has been left open!");
  }
}

void resetNotification() {
  notificationSent = 0;
  printOutput("Notified # Reset Flag");
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

void nightTime_END() {
  nightTime = 0;
  Blynk.virtualWrite(vPIN_NIGHTTIME_LED, 0);
  printOutput("Night Mode Inactive");
}


void resetDayCounters() {
  if ( (hour() == 0) && (minute() == 0) && (second() == 0) ) {
    if (!resetCountersDone) {
      GateDailyCounter = 0;
      BellDailyCounter = 0;
      Blynk.virtualWrite(vPIN_GATE_COUNTER, 0);
      Blynk.virtualWrite(vPIN_BELL_COUNTER, 0);
      resetCountersDone = 1;
    }
  }
  if ( (hour() == 0) && (minute() == 0) && (second() == 1) ) {
    resetCountersDone = 0;
  }
}

void normalSync() {
  if (year() != 1970) {
    setSyncInterval(300);
    printOutput("Sync'd RTC - Interval: 5min");
    timer.disable(timer3);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
#ifdef LOCAL_SERVER
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS, LOCAL_SERVER);
#else
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS);
#endif
  while (Blynk.connect() == false) {}
  /*********** OTA *************/
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();
  /*********** TIMERS & RTC *************/
  rtc.begin();
  setSyncInterval(5);
  timer1 = timer.setInterval(2000, sendUptime);
  timer2 = timer.setInterval(1000, ActiveGateTimer);
  timer.disable(timer2);
  timer3 = timer.setInterval(1000, normalSync);
  /******** pinModes **************/
  pinMode(PIN_DOORBELL, INPUT_PULLUP);
  pinMode(PIN_GATE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  /******** READY **************/
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("-------------");
  terminal.flush();
  /******** SYNC **************/
  Blynk.syncVirtual(vPIN_GATE_COUNTER);
  Blynk.syncVirtual(vPIN_BELL_COUNTER);
  Blynk.syncVirtual(vPIN_NOTIFY_DELAY);
  Blynk.syncVirtual(vPIN_SILENTMODE);
  Blynk.syncVirtual(vPIN_NIGHTTIME);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
  triggerBell();
  triggerGate();
  resetDayCounters();
}
/**************************************************************************************************************************/

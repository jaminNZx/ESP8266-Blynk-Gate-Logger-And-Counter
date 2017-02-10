//#define Blynk_DEBUG
//#define Blynk_PRINT Serial

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "settings.h"

WidgetTerminal terminal(vPIN_TERMINAL);
WidgetBridge base(vPIN_BRIDGE_BASE);
WidgetBridge shelf(vPIN_BRIDGE_LED);
WidgetBridge front_lights(vPIN_BRIDGE_FRONT_LIGHTS);
WidgetRTC rtc;
SimpleTimer timer;

int           DoorBellButtonCur;
byte          DoorBellButtonPrev = HIGH;
int           Gate1SwitchCurrent;        // Current state of the button
long          Gate1SwitchMillisHeld;     // How long the button was held (milliseconds)
long          Gate1SwitchSecsHeld;       // How long the button was held (seconds)
byte          Gate1SwitchPrev = LOW;
unsigned long Gate1SwitchFirstTime;       // how long since the button was first pressed
unsigned long Gate1SwitchClosedTime;
String        extraZeroH, extraZeroM, extraZeroS;
int           timer1, timer2, timer3;
int           Gate1DailyCounter, Gate2DailyCounter, BellDailyCounter;
String        Gate1LastOpened, Gate2LastOpened, BellLastPressed;
int           tableIndex1 = 0;
int           tableIndex2 = 0;
int           tableIndex3 = 0;
bool          resetCountersDone;

String getCurrentDate() {
  return String(day()) + '-' + monthShortStr(month()) + '-' + year();
}

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

void sendUptime() {
  Blynk.virtualWrite(vPIN_CUR_DATE,  getCurrentDate() + String("  ") + getCurrentTime() + String("  Wifi: ") + map(WiFi.RSSI(), -105, -40, 0, 100) + String('%') );
}

BLYNK_CONNECTED() {
  base.setAuthToken(AUTH_BASE); // Token for the Base Stations
  shelf.setAuthToken(AUTH_LIGHTS); // Token for the Smart Shelf
  front_lights.setAuthToken(AUTH_FRONT_SWITCH); // Token for the Smart Shelf
}
BLYNK_WRITE(vPIN_GATE1_COUNTER) {
  Gate1DailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_GATE2_COUNTER) {
  Gate2DailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_BELL_COUNTER) {
  BellDailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_GATE1_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_GATE1_TABLE, "clr" );
  terminal.println("Front Gate Table Cleared");
  terminal.flush();
}
BLYNK_WRITE(vPIN_GATE2_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_GATE2_TABLE, "clr" );
  terminal.println("Back Gate Table Cleared");
  terminal.flush();
}
BLYNK_WRITE(vPIN_BELL_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_BELL_TABLE, "clr" );
  terminal.println("Bell Table Cleared");
  terminal.flush();
}

void printTimeDate() {
  terminal.println("-----------------------");
  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}

void triggerBell() {
  DoorBellButtonCur = digitalRead(PIN_DOORBELL);
  if (DoorBellButtonCur == LOW && DoorBellButtonPrev == HIGH) {
    // BELL PRESSED

    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT
    Blynk.virtualWrite(vPIN_WEBHOOK_FRONT, 1);

    // SEND ALERT TO BASE STATIONS & SMART LED INTERNAL SYSTEM & FRONT LIGHTS
    base.virtualWrite(V1, HIGH);
    shelf.virtualWrite(V13, 1);
    if (hour() >= 21 || hour() <= 6) { // ONLY TRIGGER FRONT PATH LIGHTS BETWEEN 9pm-6am
      front_lights.virtualWrite(V0, 1);
    }

    // LOG WHEN
    BellLastPressed = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_BELL_LAST, BellLastPressed);

    // TERMINAL
    printTimeDate();
    terminal.println("Door Bell Pressed");
    terminal.flush();

    // DAILY COUNTER
    BellDailyCounter++;
    Blynk.virtualWrite(vPIN_BELL_COUNTER, BellDailyCounter);

    // TABLE
    Blynk.virtualWrite(vPIN_BELL_TABLE, "add", tableIndex3, BellLastPressed, 1);
    Blynk.virtualWrite(vPIN_BELL_TABLE, "pick", tableIndex3 );
    tableIndex3++;

    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
  if (DoorBellButtonCur == HIGH && DoorBellButtonPrev == LOW) {
    // perform a secondary function here
    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
}

void triggerGate1() {
  Gate1SwitchCurrent = digitalRead(PIN_GATE_SWITCH);
  if (Gate1SwitchCurrent == HIGH && Gate1SwitchPrev == LOW) {
    Gate1SwitchFirstTime = millis();
    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT
    Blynk.virtualWrite(vPIN_WEBHOOK_FRONT, 1);
    // LOG WHEN
    Gate1LastOpened = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_GATE1_LAST, Gate1LastOpened);
    // SEND ALERT TO BASE STATIONS & SMART LED INTERNAL SYSTEM & FRONT LIGHTS
    base.virtualWrite(V1, HIGH);
    shelf.virtualWrite(V13, 1);
    if (hour() >= 21 || hour() <= 6) { // ONLY TRIGGER FRONT PATH LIGHTS BETWEEN 9pm-6am
      front_lights.virtualWrite(V0, 1);
    }
    // DAILY COUNTER
    Gate1DailyCounter++;
    Blynk.virtualWrite(vPIN_GATE1_COUNTER, Gate1DailyCounter);
    // START TIMER
    timer.enable(timer2);
    // TERMINAL
    printTimeDate();
    terminal.println("Gate Opened # Timer Started");
    terminal.flush();
  }

  Gate1SwitchMillisHeld = (millis() - Gate1SwitchFirstTime);
  Gate1SwitchSecsHeld = Gate1SwitchMillisHeld / 1000;

  if (Gate1SwitchCurrent == LOW && Gate1SwitchPrev == HIGH) {
    // STOP ACTIVE GATE TIMER
    timer.disable(timer2);
    // TABLE
    Blynk.virtualWrite(vPIN_GATE1_TABLE, "add", tableIndex1, Gate1LastOpened, Gate1SwitchSecsHeld + String(" sec") );
    Blynk.virtualWrite(vPIN_GATE1_TABLE, "pick", tableIndex1 );
    tableIndex1++;
    // TERMINAL
    printTimeDate();
    terminal.println(String("Gate Closed # Timer Stopped: ") + Gate1SwitchSecsHeld + String(" sec"));
    terminal.flush();
  }
  Gate1SwitchPrev = Gate1SwitchCurrent;
}

void ActiveGate1Timer() {
  Blynk.virtualWrite(vPIN_GATE1_HELD, Gate1SwitchSecsHeld);
  if (Gate1SwitchSecsHeld == 60) {
    //Blynk.email("ben@customtabs.co.nz", "Front Gate Alert", Gate1LastOpened + " // Front Gate has been left open longer than 60 seconds!");
    //Blynk.sms("Alert! " + Gate1LastOpened + ": Gate has been left open longer than 60 seconds!");
    //Blynk.notify("Alert2, Gate1 has been left open longer than 60 seconds!");
  }
}

void resetDayCounters() {
  if ( (hour() == 0) && (minute() == 0) && (second() == 0) ) {
    if (!resetCountersDone) {
      Gate1DailyCounter = 0;
      Gate2DailyCounter = 0;
      BellDailyCounter = 0;
      Blynk.virtualWrite(vPIN_GATE1_COUNTER, 0);
      Blynk.virtualWrite(vPIN_GATE2_COUNTER, 0);
      Blynk.virtualWrite(vPIN_BELL_COUNTER, 0);
      resetCountersDone = 1;
    }
  }
  if ( (hour() == 0) && (minute() == 0) && (second() == 1) ) {
    resetCountersDone = 0;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
#if defined(USE_LOCAL_SERVER)
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS, SERVER);
#else
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS, );
#endif
  while (Blynk.connect() == false) {}
  /*********** OTA *************/
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();
  /*********** TIMERS & RTC *************/
  rtc.begin();
  timer1 = timer.setInterval(2000L, sendUptime);
  timer2 = timer.setInterval(1000L, ActiveGate1Timer);
  timer.disable(timer2);
  /******** pinModes **************/
  pinMode(PIN_DOORBELL, INPUT_PULLUP);
  pinMode(PIN_GATE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);   // DoorBell Led Pin set as output
  digitalWrite(PIN_LED, HIGH); // DoorBell Led Always On At Startup
  /******** READY **************/
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("-------------");
  terminal.flush();
  /******** SYNC **************/
  Blynk.syncVirtual(vPIN_GATE1_COUNTER);
  Blynk.syncVirtual(vPIN_GATE2_COUNTER);
  Blynk.syncVirtual(vPIN_BELL_COUNTER);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
  triggerBell();
  triggerGate1();
  resetDayCounters();
}
/**************************************************************************************************************************/

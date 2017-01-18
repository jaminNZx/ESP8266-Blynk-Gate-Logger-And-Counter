/*
  Virtual Ports:
  V0 = RTC
  V1 = Door Bell History
  V2 = Gate Open History
  V3 = Gate Timer History
  V4 = Current Date & Time
  V5 = Gate Daily Counter
  V6 = Gate LED
  V7 = Gate Last Opened
  V8 = Gate History (Sec Held Open)
  V9 = Gate Notification Indication LED
  V10 = Terminal Widget
  V11 =
  V12 =
  V13 =
  V18 = Smart Shelf Bridge
  V19 = Base Station Bridge
  V20 = Up Time (Value Widget)
  V21 = Wifi Signal (Value Widget)
*/
//#define Blynk_DEBUG
//#define Blynk_PRINT Serial
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
/**************************************************************************************************************************/
#define vPIN_RTC 0
#define vPIN_BELL_HISTORY 1
#define vPIN_GATE_HISTORY 2
#define vPIN_GATE_HELD 3
#define vPIN_CUR_DATE 4
#define vPIN_GATE_DAILY_COUNTER 5
#define vPIN_GATE_LED 6
#define vPIN_GATE_LAST 7
#define vPIN_GATE_HELD_HISTORY 8
#define vPIN_GATE_NOTIFY_LED 9
#define vPIN_TERMINAL 10
#define vPIN_TABLE 11
#define vPIN_TABLE_CLR 12
#define vPIN_LCD 13
#define vPIN_WEBHOOK 14
#define vPIN_BRIDGE_LED 18
#define vPIN_BRIDGE_BASE 19
#define vPIN_WIFI 21
/**************************************************************************************************************************/
WidgetTerminal terminal(vPIN_TERMINAL);
WidgetBridge base(vPIN_BRIDGE_BASE);
WidgetBridge shelf(vPIN_BRIDGE_LED);
WidgetRTC rtc;
BLYNK_ATTACH_WIDGET(rtc, vPIN_RTC);
SimpleTimer timer;
WidgetLCD lcd(vPIN_LCD);
/**************************************************************************************************************************/
int DoorBellButton = 5; // YELLOW
int GateSwitch = 14; // ORANGE
int DoorLed = 4; // RED
/**************************************************************************************************************************/
char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
/**************************************************************************************************************************/
int           DoorBellButtonCur;
byte          DoorBellButtonPrev = HIGH;
int           GateSwitchCurrent;        // Current state of the button
long          GateSwitchMillisHeld;     // How long the button was held (milliseconds)
long          GateSwitchSecsHeld;       // How long the button was held (seconds)
byte          GateSwitchPrev = LOW;
unsigned long GateSwitchFirstTime;       // how long since the button was first pressed
String        extraZeroH, extraZeroM, extraZeroS;
int           timer1, timer2, timer3;
int           GateDailyCounter;
String        GateLastOpened;
int           tableIndex = 0;
bool          resetGateCountDone;
int           notifyLedON;
/**************************************************************************************************************************/
String getCurrentDate() {
  return String(day()) + '-' + monthShortStr(month()) + '-' + year();
}
/**************************************************************************************************************************/
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
/**************************************************************************************************************************/
void sendUptime() {
  Blynk.virtualWrite(vPIN_CUR_DATE,  getCurrentDate() + String("  ") + getCurrentTime() + String("  Wifi: ") + map(WiFi.RSSI(), -105, -40, 0, 100) + String('%') );
}
/**************************************************************************************************************************/
void setup() {
  //Serial.begin(115200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  WiFi.mode(WIFI_STA);
  Blynk.begin(auth, "xxxxxxxxxxxxx", "xxxxxxxxxxxxx", IPAddress(192, 168, 1, 2), 8442);
  while (Blynk.connect() == false) {}
  /*********** OTA *************/
  ArduinoOTA.setHostname("Gate1");
  ArduinoOTA.begin();
  /*********** TIMERS & RTC *************/
  rtc.begin();
  timer1 = timer.setInterval(2000L, sendUptime);
  timer2 = timer.setInterval(1000L, ActiveGateTimer);
  timer3 = timer.setInterval(1000L, InactiveGateTimer);
  timer.disable(timer2);
  /******** pinModes **************/
  pinMode(DoorBellButton, INPUT_PULLUP);
  pinMode(GateSwitch, INPUT_PULLUP);
  pinMode(DoorLed, OUTPUT);   // DoorBell Led Pin set as output
  digitalWrite(DoorLed, HIGH); // DoorBell Led Always On At Startup
  /******** READY **************/
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("-------------");
  terminal.flush();
  // LCD
  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(1, 0, "Gate = Closed"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  /******** SYNC **************/
  Blynk.syncVirtual(vPIN_GATE_DAILY_COUNTER);
}
/**************************************************************************************************************************/
BLYNK_CONNECTED() {
  base.setAuthToken("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); // Token for the Base Stations
  shelf.setAuthToken("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); // Token for the Smart Shelf
}
BLYNK_WRITE(vPIN_GATE_DAILY_COUNTER) {
  GateDailyCounter = param.asInt();
}
BLYNK_WRITE(vPIN_TABLE_CLR) {
  Blynk.virtualWrite(vPIN_TABLE, "clr" );
  terminal.println("Table Cleared");
  terminal.flush();
}
/**************************************************************************************************************************/
void printTimeDate() {
  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}
/**************************************************************************************************************************/
void triggerBell() {
  DoorBellButtonCur = digitalRead(DoorBellButton);
  if (DoorBellButtonCur == LOW && DoorBellButtonPrev == HIGH) {
    // BELL PRESSED
    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT 
    Blynk.virtualWrite(vPIN_WEBHOOK, 1);
    // SEND ALERT TO BASE STATIONS
    base.virtualWrite(V1, HIGH);
    // SEND ALERT TO SMART LED INTERNAL SYSTEM
    shelf.virtualWrite(V13, 1);
    // LOG BELL PRESSED HISTORY
    Blynk.virtualWrite(vPIN_BELL_HISTORY, 1);
    printTimeDate();
    terminal.println("Door Bell Pressed");
    terminal.flush();
    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
  if (DoorBellButtonCur == HIGH && DoorBellButtonPrev == LOW) {
    // perform a secondary function here
    DoorBellButtonPrev = DoorBellButtonCur;
    delay(10);
  }
}
/**************************************************************************************************************************/
void triggerGate() {
  GateSwitchCurrent = digitalRead(GateSwitch);
  if (GateSwitchCurrent == HIGH && GateSwitchPrev == LOW && (millis() - GateSwitchFirstTime) > 200) {
    GateSwitchFirstTime = millis();
    // SEND TRIGGER TO RECORD CAMERA & CAPTURE JPEG SNAPSHOT 
    Blynk.virtualWrite(vPIN_WEBHOOK, 1);
    // SEND ALERT TO BASE STATIONS
    base.virtualWrite(V1, HIGH);
    // SEND ALERT TO SMART LED INTERNAL SYSTEM
    shelf.virtualWrite(V13, 1);
    // LOG GATE OPENED HISTORY
    Blynk.virtualWrite(vPIN_GATE_HISTORY, 1);
    // WRITE TO LED WIDGET
    Blynk.virtualWrite(vPIN_GATE_LED, 255);
    // LCD
    lcd.clear(); //Use it to clear the LCD Widget
    lcd.print(1, 0, "GATE = OPEN");
    //Blynk.setProperty(vPIN_GATE_HELD, "color", BLYNK_RED);
    // LOG WHEN
    GateLastOpened = getCurrentDate() + String("   ") + getCurrentTime();
    Blynk.virtualWrite(vPIN_GATE_LAST, GateLastOpened);
    printTimeDate();
    terminal.println("Gate Opened # Timer Started");
    terminal.flush();
    // START TIMER
    timer.disable(timer3);
    timer.enable(timer2);
    // DAILY COUNTER
    GateDailyCounter++;
    Blynk.virtualWrite(vPIN_GATE_DAILY_COUNTER, GateDailyCounter);
  }

  GateSwitchMillisHeld = (millis() - GateSwitchFirstTime);
  GateSwitchSecsHeld = GateSwitchMillisHeld / 1000;

  if (GateSwitchMillisHeld > 50) {
    if (GateSwitchCurrent == LOW && GateSwitchPrev == HIGH) {
      // WRITE TO LED WIDGET
      Blynk.virtualWrite(vPIN_GATE_LED, 0);
      // LCD
      lcd.clear(); //Use it to clear the LCD Widget
      lcd.print(1, 0, "GATE = CLOSED");
      if (GateSwitchMillisHeld >= 250) {
        // STOP ACTIVE GATE TIMER
        timer.disable(timer2);
        timer.enable(timer3);
        // LOG GATE TIMER HISTORY
        //Blynk.virtualWrite(vPIN_GATE_HELD_HISTORY, GateSwitchSecsHeld);
        // LOG GATE HISTORY TO TABLE
        Blynk.virtualWrite(vPIN_TABLE, "add", tableIndex, GateLastOpened, GateSwitchSecsHeld + String(" sec") );
        tableIndex++;
        printTimeDate();
        terminal.println(String("Gate Closed # Timer Stopped: ") + GateSwitchSecsHeld + String(" sec"));
        terminal.flush();
      }
    }
  }
  GateSwitchPrev = GateSwitchCurrent;
}
/**************************************************************************************************************************/
void ActiveGateTimer() {
  Blynk.virtualWrite(vPIN_GATE_HELD, GateSwitchSecsHeld + String(" sec"));
  if (GateSwitchSecsHeld == 60 && !notifyLedON) {
    Blynk.email("Front Gate Alert", GateLastOpened + " // Front Gate has been left open longer than 60 seconds!");
    //Blynk.sms("Alert! " + GateLastOpened + ": Gate has been left open longer than 60 seconds!");
    //Blynk.notify("Alert! " + GateLastOpened + ": Gate has been left open longer than 60 seconds!");
    Blynk.virtualWrite(vPIN_GATE_NOTIFY_LED, 255);
    notifyLedON = 1;
  }
}
void InactiveGateTimer() {
  //Blynk.virtualWrite(vPIN_GATE_HELD_HISTORY, 0);
  if (notifyLedON) {
    Blynk.virtualWrite(vPIN_GATE_NOTIFY_LED, 0);
    notifyLedON = 0;
  }
}
/**************************************************************************************************************************/
void resetGateCounter() {
  if ( (hour() == 0) && (minute() == 0) && (second() == 0) ) {
    if (!resetGateCountDone) {
      GateDailyCounter = 0;
      Blynk.virtualWrite(vPIN_GATE_DAILY_COUNTER, 0);
      resetGateCountDone = 1;
    }
  }
  if ( (hour() == 0) && (minute() == 0) && (second() == 1) ) {
    resetGateCountDone = 0;
  }
}
/**************************************************************************************************************************/
void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
  triggerBell();
  triggerGate();
  resetGateCounter();
}
/**************************************************************************************************************************/

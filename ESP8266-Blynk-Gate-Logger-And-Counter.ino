//#define Blynk_DEBUG
//#define Blynk_PRINT Serial

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#include "settings.h"
#include "globals.h"
#include "functions.h"
#include "triggerBell.h"
#include "triggerGate.h"
#include "blynk_writes.h"

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
  setSyncInterval(1);
  timer1 = timer.setInterval(2000, sendUptime);
  timer2 = timer.setInterval(1000, ActiveGateTimer);
  timer.disable(timer2);
  timer3 = timer.setInterval(1000, bootUp);
  /******** pinModes **************/
  pinMode(PIN_DOORBELL, INPUT_PULLUP);
  pinMode(PIN_GATE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  /******** READY **************/
  printOutput(String("Blynk v" BLYNK_VERSION ": Device started"));
  /******** SYNC **************/
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
}
/**************************************************************************************************************************/

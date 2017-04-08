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
  timer1 = timer.setInterval(2000, []() {
    Blynk.virtualWrite(vPIN_CUR_DATE,  getCurrentDate() + String("  ") + getCurrentTime() );
    Blynk.setProperty(vPIN_CUR_DATE, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
  });
  timer2 = timer.setInterval(1000, []() {
    Blynk.virtualWrite(vPIN_GATE_HELD, formatTime(GateSwitchMillisHeld));
    if (GateSwitchSecsHeld > 0 && GateSwitchSecsHeld >= notifyDelay) sendNotification();
  });
  timer.disable(timer2);
  timer3 = timer.setInterval(1000, []() {
    if (year() != 1970) {
      today = day();
      Blynk.syncVirtual(vPIN_GATE_COUNTER);
      Blynk.syncVirtual(vPIN_BELL_COUNTER);
      setSyncInterval(300);
      printOutput("Sync'd RTC - Interval: 5min");
      timer.disable(timer3);
      timer.setInterval(15 * 60 * 1000, []() {
        if (day() != today) {
          GateDailyCounter = 0;
          BellDailyCounter = 0;
          Blynk.virtualWrite(vPIN_GATE_COUNTER, 0);
          Blynk.virtualWrite(vPIN_BELL_COUNTER, 0);
          today = day();
        }
      });
    }
  });
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

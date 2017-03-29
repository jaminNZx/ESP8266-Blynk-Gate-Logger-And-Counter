/**************************************************************

                       General Functions

 **************************************************************/
/*
  getCurrentTime() - returns the current time as String 11:59:59
*/
String getCurrentTime() {
  String extraZeroH = "";
  String extraZeroM = "";
  String extraZeroS = "";
  if (hour() < 10) extraZeroH = '0';
  if (minute() < 10) extraZeroM = '0';
  if (second() < 10) extraZeroS = '0';
  return String(extraZeroH + hour()) + ':' + extraZeroM + minute() + ':' + extraZeroS + second();
}
/*
  getCurrentDate() - returns the current date as String DD-MM-YYYY
*/
String getCurrentDate() {
  return String(day()) + '-' + monthShortStr(month()) + '-' + year();
}
/*
  printTimeDate() - prints the current date and time to terminal with line break
*/
void printTimeDate() {
  terminal.println("-----------------------");
  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}
/*
  printOutput() - easy print to terminal with date function
*/
void printOutput(String a) {
  printTimeDate();
  terminal.println(a);
  terminal.flush();
}
/*
  sendUptime() - A 1 second interval function that updates Wifi strength meter and current time widget
*/
void sendUptime() {
  Blynk.virtualWrite(vPIN_CUR_DATE,  getCurrentDate() + String("  ") + getCurrentTime() );
  Blynk.setProperty(vPIN_CUR_DATE, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
}
/*
  resetNotification() - Reset notification flag
*/
void resetNotification() {
  notificationSent = 0;
  printOutput("Notified # Reset Flag");
}
/*
  formatTime() - Format millis in to MM:SS
*/
String formatTime(long milliSeconds) {
  long hours = 0, mins = 0, secs = 0;
  String returned, secs_o, mins_o;
  secs = milliSeconds / 1000; //convert milliseconds to secs
  mins = secs / 60; //convert seconds to minutes
  hours = mins / 60; //convert minutes to hours
  secs = secs - (mins * 60); //subtract the coverted seconds to minutes in order to display 59 secs max
  mins = mins - (hours * 60); //subtract the coverted minutes to hours in order to display 59 minutes max
  if (secs < 10 && mins) secs_o = "0";
  if (mins) returned += mins + String("m ");
  returned += secs_o + secs + String("s");
  return returned;
}
/*
  sendNotification() - sends the notification
*/
void sendNotification() {
  if (notifyDelay && !notificationSent) {
    Blynk.email("ben@customtabs.co.nz", "Front Gate Alert", GateLastOpened + String(" // Front Gate has been left open for ") + formatTime(GateSwitchMillisHeld));
    printOutput(String("Notified # Gate Held: ") + formatTime(GateSwitchMillisHeld));
    notificationSent = 1;
    timer3 = timer.setTimeout(((notifyDelay * 1000) - 1000), resetNotification);
    //Blynk.sms("Alert! " + GateLastOpened + String(" // Front Gate has been left open for ") + formatTime(GateSwitchMillisHeld);
    //Blynk.notify("NOTIFY: Alert, Front Gate has been left open!");
  }
}
/*
  ActiveGateTimer() - callback: runs while gate is open and reports the open time to the widget
*/
void ActiveGateTimer() {
  Blynk.virtualWrite(vPIN_GATE_HELD, formatTime(GateSwitchMillisHeld));
  if (GateSwitchSecsHeld > 0 && GateSwitchSecsHeld >= notifyDelay) sendNotification();
}
/*
  nightTime_END() - callback: resets nightTime mode after 10 hrs
*/
void nightTime_END() {
  nightTime = 0;
  Blynk.virtualWrite(vPIN_NIGHTTIME_LED, 0);
  printOutput("Night Mode Inactive");
}
/*
  resetDayCounters() - Reset daily counters
*/
void resetDayCounters() {
  if (day() != today) {
    GateDailyCounter = 0;
    BellDailyCounter = 0;
    Blynk.virtualWrite(vPIN_GATE_COUNTER, 0);
    Blynk.virtualWrite(vPIN_BELL_COUNTER, 0);
    today = day();
  }
}
/*
  bootUp() - A busy little run once boot function to:
    - sync time quickly
    - set the correct 'today' var 
    - sync the counters from the widget data
    - start the daily reset timer
*/
void bootUp() {
  if (year() != 1970) {
    today = day();
    Blynk.syncVirtual(vPIN_GATE_COUNTER);
    Blynk.syncVirtual(vPIN_BELL_COUNTER);
    setSyncInterval(300);
    printOutput("Sync'd RTC - Interval: 5min");
    timer.disable(timer3);
    timer.setInterval(15 * 60 * 1000, resetDayCounters);
  }
}
/*

*/

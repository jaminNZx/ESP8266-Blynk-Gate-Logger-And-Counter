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
  formatTime() - Format millis in to MM:SS
*/
String formatTime(long milliSeconds) {
  long hours = 0, mins = 0, secs = 0;
  String returned, secs_o, mins_o;
  secs = milliSeconds / 1000;
  mins = secs / 60; 
  hours = mins / 60; 
  secs = secs - (mins * 60); 
  mins = mins - (hours * 60); 
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
    timer3 = timer.setTimeout(((notifyDelay * 1000) - 1000), []() {
      notificationSent = 0;
      printOutput("Notified # Reset Flag");
    });
    //Blynk.sms("Alert! " + GateLastOpened + String(" // Front Gate has been left open for ") + formatTime(GateSwitchMillisHeld);
    //Blynk.notify("NOTIFY: Alert, Front Gate has been left open!");
  }
}

/*
  workDifference() -
*/
int workDifference(int f, int t) {
  long d = 0;
  for (int i = f; i != t; i++) d++;
  if (d > 0 && d < 24) return d;
  return d + 24;
}

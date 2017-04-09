/**************************************************************

                            Globals

 **************************************************************/
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
int           timer1, timer2, timer3, timer4;
int           GateDailyCounter, BellDailyCounter;
String        GateLastOpened, BellLastPressed;
int           tableIndex1 = 0, tableIndex2 = 0, silentBell, silentGate;
int           today, nightTime = 0;
int           nightTime_offTime = 7; // 7am turn off nightMode

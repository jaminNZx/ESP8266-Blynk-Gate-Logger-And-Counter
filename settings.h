/**************************************************************

                            Settings

 **************************************************************/
/*
     WIFI Credentials. Either use a static library, or enter the details below.
*/
 #include <wifi_credentials.h>
 //#define WIFI_SSID                  "xxxxxxxxxxxx"
 //#define WIFI_PASS                  "xxxxxxxxxxxx"
/*
     Blynk Auth Code
*/
#define AUTH                        "ae5eab51641343209ae3c2b139ef6e0b"
#define AUTH_BASE                   "8134a141d4a04faaa5a932f9606a13d2"
#define AUTH_LIGHTS                 "8e8bf31a58d843e4bc9daaaa4e7abdfd"
#define AUTH_FRONT_SWITCH           "b9af72dccede40e195093c149688c4fb"
/*
   Over The Air Hostname
*/
#define OTA_HOSTNAME                "GATE-FRONT"
/*
   Local Server Settings (uncomment to use local server)
*/
#define LOCAL_SERVER                IPAddress(192, 168, 1, 2)
/*
   Modules
*/
#define GATELOGGER
#define DOORBELL
/*
   Hardware Pins
*/
#ifdef DOORBELL
#define PIN_DOORBELL                12 
#endif
#ifdef GATELOGGER
#define PIN_GATE_SWITCH             13
#endif
/*
   Virtual Pins
*/
#define vPIN_TERMINAL               V0
#define vPIN_CUR_DATE               V1

#define vPIN_GATE_TABLE_CLR         V2
#define vPIN_BELL_TABLE_CLR         V3

#define vPIN_WEBHOOK                V4

#define vPIN_BRIDGE_LED             V5
#define vPIN_BRIDGE_BASE            V6
#define vPIN_BRIDGE_FRONT_LIGHTS    V7

#define vPIN_NOTIFY_DELAY           V8

#define vPIN_GATE_TABLE             V9
#define vPIN_GATE_LAST              V10
#define vPIN_GATE_HELD              V11
#define vPIN_GATE_COUNTER           V12

#define vPIN_BELL_LAST              V13
#define vPIN_BELL_COUNTER           V14
#define vPIN_BELL_TABLE             V15

#define vPIN_SILENTMODE             V16

#define vPIN_NIGHTTIME              V17
#define vPIN_NIGHTTIME_LED          V18
/*

*/


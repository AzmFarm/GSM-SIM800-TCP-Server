// Configure TinyGSM library
// Please select the corresponding model
// Select your modem:
#define TINY_GSM_MODEM_SIM800
#define SIM800L_IP5306_VERSION_20200811

//#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb


#include <Arduino.h>
#include "utilities.h"
#include <HardwareSerial.h> // serial(2) = pin19=RX, pin23=TX

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT  Serial1

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_GPRS true

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "apn";
const char gprsUser[] = "gprsUser";
const char gprsPass[] = "gprsPass";
//const int  port = 4001;

#include <TinyGsmClient.h>
//#include <ArduinoHttpClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

void setupModem()
{
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);

}

  void startModem() {
  // Check connection
  if (!modem.isGprsConnected()) {
    SerialMon.print("Starting IP server... ");
    // Connect to network 
    if (!modem.waitForNetwork()) {
      SerialMon.println("no network connection");
      return;
    }
    // Connect to GPRS
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println("no GPRS connection");
      return;
    }
    // Put modem in server mode
    modem.setIPserver();
    SerialMon.println("success"); // Todo: proper react on return values
    IPAddress local = modem.localIP();
    SerialMon.print("Local IP: ");
    SerialMon.println(local);
  }
}

/*void ATCommandTest() {
  //*************** Test AT command***********************
        if (SerialAT.available()) {
          SerialMon.write(SerialAT.read());
        }
        if (SerialMon.available()) {
          SerialAT.write(SerialMon.read());
        }
//*************** END Test AT command***********************
}*/
/*
void handleConnection() { // from GPRS server

  while (client.connected()) {
    if (Serial2.available()) {
      char c2 = Serial2.read();
      client.print(c2);
    }
    ATCommandTest();
  } // while (client.connected())   
}*/

void handleConnection() { // from BS RTK to Rover

//  while (client.connected()) {
    if (Serial2.available()) {
      SerialAT.print(char(Serial2.read()));
    }
    //ATCommandTest();
//  } // while (client.connected())   
}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  Serial2.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Wait...");

  // Set GSM module baud rate
    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  // Set GPS module baud rate and UART pins
    Serial2.begin(115200, SERIAL_8N1, 19, 23); //pin19=RX, pin23=TX

  setupModem();
  delay(1000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();
  delay(1000);

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) { modem.simUnlock(GSM_PIN); }
#endif
startModem();
}

void loop() {
   handleConnection();
}
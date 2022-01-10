https://github.com/vshymanskyy/TinyGSM/issues/514#issue-843535143

[x] I have read the Troubleshooting section of the ReadMe

What type of issue is this?
[ ] Request to support a new module

[ ] Bug or problem compiling the library
[ ] Bug or issue with library functionality (ie, sending data over TCP/IP)
[ ] Question or request for help
[x] Prototype for extra functionality

What are you working with?
Modem: SIM800L EVB board (v2.2 5V)
Main processor board: ESP8266 Wemos D1 pro, ESP8266 library latest (2.7.4)
TinyGSM version: Latest (0.10.9)
Code: see below

First of all thanks for making TinyGSM available, the only SIM800 library (out of many) that works for me!!

In #458 ,
user pierreverbakel had asked for the option/function in TinyGSM for the modem to function as a server.

I have the same need, so i investigated that the function is not allready available.
I investigated some more and came up with this solution to get it in, and for me it is, sort of, working.

So i am handing it to the project.
But i am not an advanced programmer, the TinyGSM code is not even clear to me all the time.
And i am newby on Github, no experience in pull requesting etc.
Some work still probably needs to be done to make the code 'proper'.
I am sure this can be easily be picked up by the maintainers, hence this request.

In my project with and SIM800L EVB board (the very cheap one) this works, sort of.
Sort of, because regularly (several times per hour) the connection is dropped for reasons i can not find (as of yet).

I see that per #481 this is a known problem, so perhaps we need to live with it?

As a work around, i call on function startModem() every minute in my loop to check and, if needed, reconnect.
This will sometimes lead to a missed request, and a minute time to respond again after loss of connection.

Here is what i did:
//********************In TinyGsmModem.tpp put:******************************************

  bool setIPserver() { // put modem in server mode
    return thisModem().setIPserverImpl();
  }

  bool unsetIPserver() { // get modem out of server mode
    return thisModem().unsetIPserverImpl();
  }
//***********************In TinyGsmClientSIM800.h put:***************************************

  bool setIPserverImpl() { // Todo: return values on errors or success
     sendAT(GF("+CIPSERVER=1,80")); // put modem in server mode
     waitResponse();

  }
  bool unsetIPserverImpl() { // Todo: return values on errors or success
     sendAT(GF("+CIPSERVER=0")); // put modem in server mode
     waitResponse();
  }

//*****************************************************************************************************************
    //Set to multi-IP AT+CIPMUX=<n>, <n>=0 Single IP connection, <n>=1 Multi IP connection;
    sendAT(GF("+CIPMUX=0"));
    if (waitResponse() != 1) { return false; } 
	
	// Set to AT+CIPMODE=<n>, <n>=0 NORMAL MODE (AT Command pushed mode),<n>=1 -TRANSPARENT MODE, if CIPMODE=1 then CIPMUX=0
    sendAT(GF("+CIPMODE=1"));
    if (waitResponse() != 1) { return false; } 
//*****************************************************************************************************************	

//***********************Now in your program you can do:***************************************

Function startModem:

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
    SerialMon.print("Local IP: ");
    SerialMon.println(modem.localIP());
  }
}

Function handleConnection:

void handleConnection() { // from GPRS server
  // Get available data
  String request = "";
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      request.concat(c);
    }
    //SerialMon.println(request);
    // Do something with request
    String reqAction = request.substring(0,3);
    String reqpage = request.substring(request.indexOf('/'), request.indexOf('/') + 10);
    if (reqAction == "GET") { // load page data       
      SerialMon.print("GET: ");
      SerialMon.println(reqpage);
      if (reqpage == "/requestedpage") {
        // Your reply to GET
        client.print("HTTP/1.1 200 OK\r\n");
        client.print("Access-Control-Allow-Origin: *\r\n");
        client.print("Content-Type: text/plain\r\n");  
        client.print("Connection: Close\r\n");
        client.println();
        client.println("Hello World");
        client.println();
        client.stop();
      }
    } else { 
      SerialMon.println("Request from gprs client not clear");
      client.stop();
    }
    request = "";
  } // while (client.connected())   
}

//***********************In setup:***************************************

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(5000);

  // Initialise modem
  SerialMon.println("Initializing modem...");
  modem.restart();
  delay(1000);
  
  startModem();
  //....

//****************In loop:**********************************

handleConnection();

//**********************C:\Users\azamat\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.2\cores\esp32\HardwareSerial.cpp****************************
https://www.youtube.com/watch?v=GwShqW39jlE&t=130s

#if SOC_UART_NUM > 2
#ifndef RX2
#if CONFIG_IDF_TARGET_ESP32
//#define RX2 16 // RX GPIO16 default
#define RX2 4 // RX GPIO4
#endif
#endif

#ifndef TX2
#if CONFIG_IDF_TARGET_ESP32
//#define TX2 17 // TX GPIO17 default
#define TX2 0 // TX GPIO0
#endif
#endif
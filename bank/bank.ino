// Your GPRS credentials (leave empty, if not needed)
const char apn[]      = "internet"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "true"; // GPRS User
const char gprsPass[] = "true"; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = ""; 

const int  port = 443;                             // server port number

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";
#include <HTTPClient.h>
// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22
// BME280 pins
#define I2C_SDA_2            18
#define I2C_SCL_2            19

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif




//Your Domain name with URL path or IP address with path
const String serverName = "https://smartbus-7lpin5zc7a-as.a.run.app/api/v1/signage/2/arrival?fbclid=IwAR3F0wzVWvxdhmus2AgApiyQYMbfM9Hh2Pcc52oQ2xuB-nIqZyRObdzrMjk";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

void setup() {
  SerialMon.begin(115200); 
    pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  SerialMon.println("Initializing modem...");
 
  // use modem.init() if you don't need the complete restart
  modem.init();
   if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }

 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  //Send an HTTP POST request every 10 minutes
 SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }
  else {
    SerialMon.println(" OK");
    
    SerialMon.print("Connecting to ");
    SerialMon.print(serverName);
//    if (!client.connect(serverName, port)) {
//      SerialMon.println(" fail");
//    }
//    else {
      SerialMon.println(" OK");
      HTTPClient http;

      String serverPath = serverName + "?temperature=24.37";
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        SerialMon.print("HTTP Response code: ");
        SerialMon.println(httpResponseCode);
        String payload = http.getString();
        SerialMon.println(payload);
      }
      else {
        SerialMon.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
//    }
//    else {
//      Serial.println("WiFi Disconnected");
//    }
//    lastTime = millis();
  }
}

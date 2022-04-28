// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_SIM5360
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH
// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1
// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX
// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#define TINY_GSM_RX_BUFFER 1024

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS
// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
// Add a reception delay - may be needed for a fast processor at a slow baud rate
// #define TINY_GSM_YIELD() { delay(2); }
// Uncomment this if you want to use SSL
#define USE_SSL
// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
// set GSM PIN, if any
#define GSM_PIN ""
// Your GPRS credentials, if any

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

const char apn[] = "internet";
const char gprsUser[] = "true";
const char gprsPass[] = "true";
// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "mi10tpro";
const char wifiPass[] = "123456789";
// Server details
const char server[] = "console.firebase.google.com";
const char resource[] = "/u/0/project/smartbut-8e052/database/smartbut-8e052-default-rtdb/data";

//const char server[]   = "www.google.com";
//const char resource[] = "/";
const int  port       = 443;

#include <Wire.h>

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
 // Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS#include <StreamDebugger.h>

StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
#ifdef USE_SSL
TinyGsmClientSecure client(modem);
//const int port = 443;
#else
TinyGsmClientSecure client(modem);
//const int port = 80;
#endif

//HttpClient  http(client, server, port);
String apiKeyValue = "EHfdf2KtFWC5WaizFIk8pxOQ9L3bxYgmLmAnR6Bg";
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22

TwoWire I2CPower = TwoWire(0);

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);
  delay(10);
  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!
  SerialMon.println("Wait...");
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  // Set GSM module baud rate
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  // SerialAT.begin(9600);
  delay(6000);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
  #if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }
  #endif
}
void loop() {
  #if TINY_GSM_USE_WIFI
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  #endif
  #if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);
  #endif
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }
  #if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
  #endif
//  SerialMon.print("Connecting to ");
//  SerialMon.println(server);
//  if (!client.connect(server, port)) {
//    SerialMon.println(" fail");
//    delay(10000);
//    return;
//  }
//  SerialMon.println(" success");
//  // Make a HTTP GET request:
//  String payload = client.readString();
//  Serial.println(payload);
//   client +=+ FIREBASE_AUTH;
  HttpClient  http(client, server, port);

 SerialMon.print(F("Performing HTTPS GET request... "));
  http.connectionKeepAlive();  // Currently, this is needed for HTTPS
  SerialMon.println(resource);
  int err = http.get(resource);
  if (err != 0) {
    SerialMon.println(F("failed to connect1"));
    SerialMon.println(err);
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print(F("Response status code: "));
  SerialMon.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  SerialMon.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName  = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }
  if (http.isResponseChunked()) {
    SerialMon.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  // Shutdown

  http.stop();
  SerialMon.println(F("Server disconnected"));

  
  SerialMon.println();
  // Shutdown
  client.stop();
  SerialMon.println(F("Server disconnected"));
  #if TINY_GSM_USE_WIFI
  modem.networkDisconnect();
  SerialMon.println(F("WiFi disconnected"));
  #endif
  #if TINY_GSM_USE_GPRS
  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
  #endif
  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

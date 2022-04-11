
// ----------------------------
// Standard Libraries
// ----------------------------

#include <jsonlib.h>
// Your GPRS credentials (leave empty, if not needed)
const char apn[]      = "internet"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "true "; // GPRS User
const char gprsPass[] = "true "; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = ""; 
// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

////------- Replace the following! ------
//char ssid[] = "mi10tpro";       // your network SSID (name)
//char password[] = "123456789";  // your network key

// For Non-HTTPS requests
// WiFiClient client;
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>
#ifdef DUMP_AT_COMMANDS
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif
// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
// For HTTPS requests
//WiFiClientSecure client;
// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

// Just the base of the URL you want to connect to
#define TEST_HOST "smartbus-7lpin5zc7a-as.a.run.app"

// OPTIONAL - The finferprint of the site you want to connect to.
//#define TEST_HOST_FINGERPRINT "7c4c2fe884ff09a8f486dec403ea5d6793c24baa"
// The finger print will change every few months.


void setup() {

  Serial.begin(115200);
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  Serial.println("Initializing modem...");
  modem.restart();
  // use modem.init() if you don't need the complete restart
  modem.init();
  // Connect to the WiFI
   if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }
//  WiFi.mode(WIFI_STA);
//  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting apn: ");
  Serial.print(apn);
  Serial.print("Connecting to ");
  Serial.print(TEST_HOST);
//    if (!client.connect(TEST_HOST, port)) {
//      Serial.println(" fail");
//    }
//  Serial.println(ssid);
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
//    delay(500);
//  }
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
//  IPAddress ip = WiFi.localIP();
//  Serial.println(ip);

  //--------

  // If you don't need to check the fingerprint
  // client.setInsecure();

  // If you want to check the fingerprint
//  client.setFingerprint(TEST_HOST_FINGERPRINT);


}

void makeHTTPRequest() {

  // Opening connection to server (Use 80 as port if HTTP)
  if (!client.connect(TEST_HOST, 443))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  // give the esp a breather
  yield();

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  client.print("/api/v1/signage/2/arrival?fbclid=IwAR0nsIosXG3b-clGm0tdmfStC8ZFTFsUb_rrPsikMAoIy7hiC8S_xhvkL_s"); // %2C == ,
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }
  //delay(100);
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  // This is probably not needed for most, but I had issues
  // with the Tindie api where sometimes there were random
  // characters coming back before the body of the response.
  // This will cause no hard to leave it in
  // peek() will look at the character, but not take it off the queue
  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
    Serial.println("BAD");
  }
  String payload = client.readString();
  Serial.println(payload);

  String arrival = jsonExtract(payload, "arrival");
  Serial.println(arrival);
}

void loop() {
  // put your main code here, to run repeatedly:
  makeHTTPRequest();
  delay(30000);
}

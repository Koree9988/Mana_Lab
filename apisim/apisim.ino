const char apn[]      = "internet"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "true "; // GPRS User
const char gprsPass[] = "true "; // GPRS Password
const char simPIN[] = "";
const char server[] = "smartbus-7lpin5zc7a-as.a.run.app";
const int port = 443;
// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define SerialMon Serial
#define SerialAT Serial1
#define TIMEOUT  5000 
// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data
#include <TinyGsmClient.h>
#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00
unsigned long lastConnectionTime = 0; // Track the last connection time
unsigned long lastUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 60L * 1000L; // Post data every 2 minutes
const unsigned long updateInterval = 10L * 1000L; // Update once every 10 seconds
void setup() {
SerialMon.begin(115200);
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);
  SerialMon.println("Initializing modem...");
  modem.init();
  // Configure the wake up source as timer wake up  
  
  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }
  else {
    SerialMon.println(" OK");
    
    SerialMon.print("Connecting to ");
    SerialMon.print(server);
    if (!client.connect(server, port)) {
    
      SerialMon.println(" Fail");
    }
    else {
      SerialMon.println(" OK");
    }
  }
}
void loop() {
  // If update time has reached 1 second, then update the jsonBuffer
  if (millis() - lastUpdateTime >=  updateInterval) {
    updatesJson(jsonBuffer);
  }
    
}
// Updates the josnBuffer with data
void updatesJson(char* jsonBuffer){
  /* JSON format for updates paramter in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "[{\"delta_t\":0,\"field1\":-70},{\"delta_t\":3,\"field1\":-66}]"
   */
  // Format the jsonBuffer as noted above
  strcat(jsonBuffer,"{\"delta_t\":");
  unsigned long deltaT = (millis() - lastUpdateTime)/1000;
  size_t lengthT = String(deltaT).length();
  char temp[4];
  String(deltaT).toCharArray(temp,lengthT+1);
  strcat(jsonBuffer,temp);
  strcat(jsonBuffer,",");
  int hall = hallRead(); 
  strcat(jsonBuffer, "\"field1\":");
  lengthT = String(hall).length();
  String(hall).toCharArray(temp,lengthT+1);
  strcat(jsonBuffer,temp);
  strcat(jsonBuffer,"},");
  // If posting interval time has reached 2 minutes, update the ThingSpeak channel with your data
  if (millis() - lastConnectionTime >=  postingInterval) {
        size_t len = strlen(jsonBuffer);
        jsonBuffer[len-1] = ']';
        httpRequest(jsonBuffer);
  }
  lastUpdateTime = millis(); // Update the last update time
}
// Updates the ThingSpeakchannel with data
void httpRequest(char* jsonBuffer) {
  /* JSON format for data buffer in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "{\"write_api_key\":\"YOUR-CHANNEL-WRITEAPIKEY\",\"updates\":[{\"delta_t\":0,\"field1\":-60},{\"delta_t\":15,\"field1\":200},{\"delta_t\":15,\"field1\":-66}]
   */
  // Format the data buffer as noted above
  char data[500] = "{\"write_api_key\":\"SUMY3MRLMBK2HEKK\",\"updates\":"; // Replace YOUR-CHANNEL-WRITEAPIKEY with your ThingSpeak channel write API key
  strcat(data,jsonBuffer);
  strcat(data,"}");
  // Close any connection before sending a new request
  client.stop();
  String data_length = String(strlen(data)+1); //Compute the data buffer length
  Serial.println(data);
  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    client.println("POST /channels/1300373/bulk_update.json HTTP/1.1"); // Replace YOUR-CHANNEL-ID with your ThingSpeak channel ID
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: mw.doc.bulk-update (Arduino ESP8266)");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("Content-Length: "+data_length);
    client.println();
    client.println(data);
  }
  else {
    Serial.println("Failure: Failed to connect to ThingSpeak");
  }
  //delay(250); //Wait to receive the response
  //client.parseFloat();
  //String resp = String(client.parseInt());
  //Serial.println("Response code:"+resp); // Print the response code. 202 indicates that the server has accepted the response
  jsonBuffer[0] = '['; //Reinitialize the jsonBuffer for next batch of data
  jsonBuffer[1] = '\0';
  lastConnectionTime = millis(); //Update the last conenction time
}

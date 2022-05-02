#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

//Your IP address or domain name with URL path
const char* serverNameTemp = "http://192.168.4.1/arrival";

#include <Wire.h>

String arrival;

unsigned long previousMillis = 0;
const long interval = 5000; 


// Header file includes
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_CLK_ZONES 2
#define ZONE_SIZE 4
#define MAX_DEVICES (MAX_CLK_ZONES * ZONE_SIZE)

#define ZONE_UPPER  1
#define ZONE_LOWER  0

#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D4

#if	USE_DS1307
#include <MD_DS1307.h>
#include <Wire.h>
#endif

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define SPEED_TIME  75
#define PAUSE_TIME  0

#define MAX_MESG  6

// Hardware adaptation parameters for scrolling
bool invertUpperZone = false;

// Turn on debug statements to the serial output
#define  DEBUG  0

// Global variables
char  szTimeL[MAX_MESG];    // mm:ss\0
char  szTimeH[MAX_MESG];

void getTime(char *psz, bool f = true)
// Code for reading clock time
// Simulated clock runs 1 minute every seond
{
  uint16_t  h, m;

  m = millis()/1000;
  h = (m/60) % 24;
  m %= 60;
  sprintf(psz, "%s", arrival);
}

void createHString(char *pH, char *pL)
{
  for (; *pL != '\0'; pL++)
    *pH++ = *pL | 0x80;   // offset character

  *pH = '\0'; // terminate the string
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  
  invertUpperZone = (HARDWARE_TYPE == MD_MAX72XX::GENERIC_HW || HARDWARE_TYPE == MD_MAX72XX::PAROLA_HW);

  // initialise the LED display
  P.begin(MAX_CLK_ZONES);

  // Set up zones for 2 halves of the display
  P.setZone(ZONE_LOWER, 0, ZONE_SIZE - 1);
  P.setZone(ZONE_UPPER, ZONE_SIZE, MAX_DEVICES - 1);
  P.setFont(numeric7SegDouble);

  P.setCharSpacing(P.getCharSpacing() * 2); // double height --> double spacing

  if (invertUpperZone)
  {
    P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_UD);
    P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_LR);

    P.displayZoneText(ZONE_LOWER, szTimeL, PA_RIGHT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_UPPER, szTimeH, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  }
  else
  {
    P.displayZoneText(ZONE_LOWER, szTimeL, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_UPPER, szTimeH, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  }

}

void loop(void)
{
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval) {
     // Check WiFi connection status
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      arrival = httpGETRequest(serverNameTemp);
      Serial.println("arrival: " + arrival);
      
      
      // save the last HTTP GET Request
      previousMillis = currentMillis;
    }
    else {
      Serial.println("WiFi Disconnected");
      Serial.print("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("Connected to WiFi");
    }
  }
  static uint32_t	lastTime = 0; // millis() memory
  static bool	flasher = false;  // seconds passing flasher

  P.displayAnimate();
  if (P.getZoneStatus(ZONE_LOWER) && P.getZoneStatus(ZONE_UPPER))
  {
    // Adjust the time string if we have to. It will be adjusted
    // every second at least for the flashing colon separator.
    if (millis() - lastTime >= 1000)
    {
      lastTime = millis();
      getTime(szTimeL, flasher);
      createHString(szTimeH, szTimeL);
      flasher = !flasher;

      P.displayReset();

      // synchronise the start
      P.synchZoneStart();
    }
  }
}


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

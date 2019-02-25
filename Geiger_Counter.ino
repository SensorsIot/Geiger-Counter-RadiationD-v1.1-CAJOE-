/*
   Geiger.ino

   This code interacts with the Alibaba RadiationD-v1.1 (CAJOE) Geiger counter board
   and reports readings in CPM (Counts Per Minute).

   Author: Andreas Spiess

   Based on initial work of Mark A. Heckler (@MkHeck, mark.heckler@gmail.com)

   License: MIT License

   Please use freely with attribution. Thank you!
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IFTTTMaker.h>
#include <SSD1306.h>
#include <credentials.h> // or define mySSID and myPASSWORD and THINGSPEAK_API_KEY

#define LOG_PERIOD 20000 //Logging period in milliseconds
#define MINUTE_PERIOD 60000
#define WIFI_TIMEOUT_DEF 30

#ifndef CREDENTIALS
// WLAN
#define mySSID "SSID"
#define myPASSWORD "password"

//Thinspeak
#define THINKSPEAK_CHANNEL 123456
#define WRITE_API_KEY  "key"

// IFTTT
#define IFTTT_KEY "......."

#endif

// IFTTT
#define EVENT_NAME "Radioactivity" // Name of your event name, set when you are creating the applet

// ThingSpeak Settings
const int channelID = THINKSPEAK_CHANNEL;
const char* server = "api.thingspeak.com";


char ssid[] = mySSID;

IPAddress ip;

WiFiClient client;

WiFiClientSecure secure_client;
IFTTTMaker ifttt(IFTTT_KEY, secure_client);
SSD1306  display(0x3c, 5, 4);


volatile unsigned long counts = 0;                       // Tube events
unsigned long cpm = 0;                                   // CPM
unsigned long previousMillis;                            // Time measurement
const int inputPin = 26;

void ISR_impulse() { // Captures count of events from Geiger counter board
  counts++;
}

void setup() {
  Serial.begin(115200);
  displayInit();
  displayString("Welcome", 64, 15);
  Serial.println("Connecting to Wi-Fi");

  WiFi.begin(mySSID, myPASSWORD);

  int wifi_loops=0;
  int wifi_timeout = WIFI_TIMEOUT_DEF;
  while (WiFi.status() != WL_CONNECTED) {
    wifi_loops++;
    Serial.print(".");
    delay(500);
    if (wifi_loops>wifi_timeout)
      {
        software_Reset();
      }
  }
  Serial.println();
  Serial.println("Wi-Fi Connected");
  display.clear();
  displayString("Measuring", 64, 15);
  pinMode(inputPin, INPUT);                                                // Set pin for capturing Tube events
  interrupts();                                                            // Enable interrupts
  attachInterrupt(digitalPinToInterrupt(inputPin), ISR_impulse, FALLING); // Define interrupt on falling edge
}

void loop() {
	
  unsigned long currentMillis = millis();
  
  if (WiFi.status() != WL_CONNECTED)
  {
	  software_Reset();
	}

  if (currentMillis - previousMillis > LOG_PERIOD) {
    previousMillis = currentMillis;
    cpm = counts * MINUTE_PERIOD / LOG_PERIOD;
    //cpm=105;
    counts = 0;
    display.clear();
    displayString("Radioactivity", 64, 0);
    displayInt(cpm, 64, 30);
    postThinspeak(cpm);
    if (cpm > 100 ) IFTTT( EVENT_NAME, cpm);
	
  }
}

void postThinspeak(int postValue) {
  if (client.connect(server, 80)) {

    // Construct API request body
    String body = "field1=";
    body += String(postValue);

    Serial.print("Radioactivity (CPM): ");
    Serial.println(postValue);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(WRITE_API_KEY) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");

  }
  client.stop();
}

void IFTTT(String event, int postValue) {
  if (ifttt.triggerEvent(EVENT_NAME, String(postValue))) {
    Serial.println("Successfully sent to IFTTT");
  } else
  {
    Serial.println("IFTTT failed!");
  }
}

void displayInit() {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
}

void displayInt(int dispInt, int x, int y) {
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(x, y, String(dispInt));
  display.setFont(ArialMT_Plain_24);
  display.display();
}

void displayString(String dispString, int x, int y) {
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(x, y, dispString);
  display.setFont(ArialMT_Plain_24);
  display.display();
}


/****reset***/
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
Serial.print("resetting");
esp_restart(); 
}

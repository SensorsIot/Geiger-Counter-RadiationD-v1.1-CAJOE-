/*
   Geiger.ino

   This code interacts with the Alibaba RadiationD-v1.1 (CAJOE) Geiger counter board
   and reports readings in CPM (Counts Per Minute).
   Connect the output of the Geiger counter to pin inputPin.

   ******* !!!!!!!!! Please use Arduino JSON Library < version 6.0
   Install Thingspulse SSD1306 Library

   Author: Andreas Spiess
   Based on initial work of Mark A. Heckler (@MkHeck, mark.heckler@gmail.com)
   License: MIT License
   Please use freely with attribution. Thank you!
*/

#define PRINT_DEBUG_MESSAGES

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IFTTTMaker.h>
#include <ThingSpeak.h>
#include <SSD1306.h>
#include <credentials.h>                 // or define mySSID and myPASSWORD and THINGSPEAK_API_KEY

#define WIFI_TIMEOUT_DEF 30
#define PERIOD_LOG 5                  //Logging period 
#define PERIOD_THINKSPEAK 60        // in seconds, >60

#ifndef CREDENTIALS
// WLAN
#define mySSID ""
#define myPASSWORD ""

//IFTT
#define IFTTT_KEY "......."

// Thingspeak
#define SECRET_CH_ID 000000      // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "XYZ"   // replace XYZ with your channel write API Key

#endif

// IFTTT
#define EVENT_NAME "Radioactivity" // Name of your event name, set when you are creating the applet


IPAddress ip;

WiFiClient client;

WiFiClientSecure secure_client;
IFTTTMaker ifttt(IFTTT_KEY, secure_client);
SSD1306  display(0x3c, 5, 4);

const int inputPin = 17;

volatile unsigned long counts = 0;                       // Tube events
int cpm = 0;                                             // CPM
int lastCounts = 0;
unsigned long lastCountTime;                            // Time measurement
unsigned long lastEntryThingspeak;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

void IRAM_ATTR ISR_impulse() { // Captures count of events from Geiger counter board
  counts++;
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


void IFTTT(String event, int postValue) {
  if (ifttt.triggerEvent(EVENT_NAME, String(postValue))) {
    Serial.println("Successfully sent to IFTTT");
  } else
  {
    Serial.println("IFTTT failed!");
  }
}

void postThingspeak( int value) {
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeField(myChannelNumber, 1, value, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

void setup() {
  Serial.begin(115200);

  if (PERIOD_LOG > PERIOD_THINKSPEAK) {
    Serial.println("PERIOD_THINKSPEAK has to be bigger than PERIODE_LOG");
    while (1);
  }
  displayInit();
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  displayString("Welcome", 64, 15);
  Serial.println("Connecting to Wi-Fi");

  WiFi.begin(mySSID, myPASSWORD);

  int wifi_loops = 0;
  int wifi_timeout = WIFI_TIMEOUT_DEF;
  while (WiFi.status() != WL_CONNECTED) {
    wifi_loops++;
    Serial.print(".");
    delay(500);
    if (wifi_loops > wifi_timeout) software_Reset();
  }
  Serial.println();
  Serial.println("Wi-Fi Connected");
  display.clear();
  displayString("Measuring", 64, 15);
  pinMode(inputPin, INPUT);                            // Set pin for capturing Tube events
  attachInterrupt(inputPin, ISR_impulse, FALLING);     // Define interrupt on falling edge
  lastEntryThingspeak = millis();
  lastCountTime = millis();
  Serial.println("Initialized");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) software_Reset();

  if (millis() - lastCountTime > (PERIOD_LOG * 1000)) {
    Serial.print("Counts: "); Serial.println(counts);
    cpm = (counts - lastCounts) / PERIOD_LOG ;
    lastCounts = counts;
    lastCountTime = millis();
    display.clear();
    displayString("Radioactivity", 64, 0);
    displayInt(cpm, 64, 30);
    if (cpm > 100 ) IFTTT( EVENT_NAME, cpm);
    Serial.print("cpm: "); Serial.println(cpm);
  }

  if (millis() - lastEntryThingspeak > (PERIOD_THINKSPEAK * 1000)) {
    Serial.print("Counts: "); Serial.println(counts);
    int averageCPM = (counts) / PERIOD_THINKSPEAK;
    postThingspeak(averageCPM);
    lastEntryThingspeak = millis();
    counts=0;
    lastCounts=0;
  }
}

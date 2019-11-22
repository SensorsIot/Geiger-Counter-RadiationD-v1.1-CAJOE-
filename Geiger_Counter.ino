/*
   edited by genewitch to actually provide clicks per minute on the serial port
   in a sane fashion. the original file i forked from just set CPM to 105 and
   didn't really work anyhow, as the count was reset every 20 seconds.
   There's an issue with my understanding of the way variables work, so please
   feel free to fix my kludge with the clock1 and start variables. when they
   were in the loop the "minutes" never changed.
*/

/*
   Geiger.ino

   This code interacts with the Alibaba RadiationD-v1.1 (CAJOE) Geiger counter board
   and reports readings in CPM (Counts Per Minute).

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
#include <credentials.h> // or define mySSID and myPASSWORD and THINGSPEAK_API_KEY

#define LOG_PERIOD 20000 //Logging period in milliseconds
#define MINUTE_PERIOD 60000
#define WIFI_TIMEOUT_DEF 30
#define PERIODE_THINKSPEAK 20000

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

volatile unsigned long counts = 0;                       // Tube events
unsigned long cpm = 0;                                   // CPM
unsigned long previousMillis;                            // Time measurement
const int inputPin = 7;
unsigned int thirds = 0;
unsigned long minutes = 1;
unsigned long start = 0;
unsigned long entryThingspeak;
unsigned long currentMillis = millis();

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

#define LOG_PERIOD 20000 //Logging period in milliseconds
#define MINUTE_PERIOD 60000

void ISR_impulse() { // Captures count of events from Geiger counter board
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
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

void setup() {
  Serial.begin(115200);
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
    if (wifi_loops > wifi_timeout)
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
  unsigned long clock1 = millis();
  start = clock1;
}

void loop() {

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
    if (cpm > 100 ) IFTTT( EVENT_NAME, cpm);
  }
  // Serial.print("minutes: ");
  // Serial.println(String(minutes));

  //cpm = counts * MINUTE_PERIOD / LOG_PERIOD; this is just counts times 3 so:

  cpm = counts / minutes;


  if (millis() - entryThingspeak > PERIODE_THINKSPEAK) {
    Serial.print("Total clicks since start: ");
    Serial.println(String(counts));
    Serial.print("Rolling CPM: ");
    Serial.println(String(cpm));
    postThingspeak(cpm);
    entryThingspeak = millis();
  }

  //    if ( thirds > 2) {
  //      counts = 0;
  //      thirds = 0;
  //    }
}

 /*
   Geiger.ino

   This code interacts with the Alibaba RadiationD-v1.1 (CAJOE) Geiger counter board
   and reports on Display readings in CPM (Counts Per Minute).
   To the Thingspeak CPH (Counts Per Hour) are reported
   Connect the output of the Geiger counter to pin inputPin.

   Install Thingspulse SSD1306 Library IFTTTWebhook and ThingSpeak
   Please notice you need to udate the https cerificatet signere certificate within IFTTTWebhook - doues not work with certificate manager.
   IFTTTWebhook does not work with parameter .…… i asume as results from api changes.
   Examle of my deice https://thingspeak.com/channels/1754536#

   Author Hans Carlos Hofmann
   Based on Andreas Spiess
   Based on initial work of Mark A. Heckler (@MkHeck, mark.heckler@gmail.com)
   License: MIT License
   Please use freely with attribution. Thank you!
*/

#define Version "V1.3.1"
#define CONS
#define WIFI
#define IFTT
#ifdef CONS
#define PRINT_DEBUG_MESSAGES
#endif

#ifdef WIFI
#include <WiFi.h>
#include <WiFiClientSecure.h>
#ifdef IFTT
#include "IFTTTWebhook.h"
#endif
#include <ThingSpeak.h>
#endif
#include <SSD1306.h>
#include <esp_task_wdt.h>

// #include <credentials.h>                 // or define mySSID and myPASSWORD and THINGSPEAK_API_KEY

#define WIFI_TIMEOUT_DEF 30
#define PERIOD_LOG 15                //Logging period 
#define PERIOD_THINKSPEAK 3600        // in seconds, >60
#define WDT_TIMEOUT 10

#ifndef CREDENTIALS
// WLAN
#ifdef WIFI
#define mySSID "xxx"
#define myPASSWORD "xxx"

//IFTT
#ifdef IFTT
#define IFTTT_KEY "xxx"
#endif

// Thingspeak
#define SECRET_CH_ID 00000000      // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "xxx"   // replace XYZ with your channel write API Key

#endif

// IFTTT
#ifdef IFTT
#define EVENT_NAME "Radioactivity" // Name of your event name, set when you are creating the applet
#endif
#endif

IPAddress ip;

#ifdef WIFI
WiFiClient client;

WiFiClientSecure secure_client;
#endif

SSD1306  display(0x3c, 5, 4);

const int inputPin = 26;

int counts = 0;  // Tube events
int counts2 = 0;
int cpm = 0;                                             // CPM
unsigned long lastCountTime;                            // Time measurement
unsigned long lastEntryThingspeak;
unsigned long startCountTime;                            // Time measurement
unsigned long startEntryThingspeak;

#ifdef WIFI
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
#endif


void IRAM_ATTR ISR_impulse() { // Captures count of events from Geiger counter board
      counts++;
      counts2++;
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
#ifdef CONS
  Serial.println("resetting by software");
#endif
  displayString("Myreset", 64, 15);
  delay(1000);
  esp_restart();
}


void IFTTT(int postValue) {
#ifdef WIFI
#ifdef IFTT
  IFTTTWebhook webhook(IFTTT_KEY, EVENT_NAME);
  if (!webhook.trigger(String(postValue).c_str())) {
#ifdef CONS
    Serial.println("Successfully sent to IFTTT");
  } else
  {
    Serial.println("IFTTT failed!");
#endif
  }
#endif
#endif
}

void postThingspeak( int value) {
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
#ifdef WIFI
  int x = ThingSpeak.writeField(myChannelNumber, 1, value, myWriteAPIKey);
#ifdef CONS
  if (x == 200) {
    Serial.println("Channel update successful");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
#endif
#endif
}

void printStack()
{
#ifdef CONS
  char *SpStart = NULL;
  char *StackPtrAtStart = (char *)&SpStart;
  UBaseType_t watermarkStart = uxTaskGetStackHighWaterMark(NULL);
  char *StackPtrEnd = StackPtrAtStart - watermarkStart;
  Serial.printf("=== Stack info === ");
  Serial.printf("Free Stack is:  %d \r\n", (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
#endif
}

void setup() {
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  
#ifdef CONS
  Serial.begin(115200);
  Serial.print("This is ") ; Serial.println(Version) ;
#endif 

  if (PERIOD_LOG > PERIOD_THINKSPEAK) {
#ifdef CONS
    Serial.println("PERIOD_THINKSPEAK has to be bigger than PERIODE_LOG");
#endif
    while (1);
  }
  displayInit();
#ifdef WIFI
  ThingSpeak.begin(client);  // Initialize ThingSpeak
#endif
  displayString("Welcome", 64, 0);
  displayString(Version, 64, 30);
  printStack();
#ifdef WIFI
#ifdef CONS
  Serial.println("Connecting to Wi-Fi");
#endif 

  WiFi.begin(mySSID, myPASSWORD);

  int wifi_loops = 0;
  int wifi_timeout = WIFI_TIMEOUT_DEF;
  while (WiFi.status() != WL_CONNECTED) {
    wifi_loops++;
#ifdef CONS
    Serial.print(".");
#endif
    delay(500);
    if (wifi_loops > wifi_timeout) software_Reset();
  }
#ifdef CONS
  Serial.println();
  Serial.println("Wi-Fi Connected");
#endif
#endif
  display.clear();
  displayString("Measuring", 64, 15);
  pinMode(inputPin, INPUT);                            // Set pin for capturing Tube events
#ifdef CONS
  Serial.println("Defined Input Pin");
#endif
  attachInterrupt(digitalPinToInterrupt(inputPin), ISR_impulse, FALLING);     // Define interrupt on falling edge
  Serial.println("Irq installed");

  startEntryThingspeak = lastEntryThingspeak = millis();
  startCountTime = lastCountTime = millis();
#ifdef CONS
  Serial.println("Initialized");
#endif  
}

int active = 0 ;

void loop() {
  esp_task_wdt_reset();
#ifdef WIFI
  if (WiFi.status() != WL_CONNECTED) software_Reset();
#endif

  if (millis() - lastCountTime > (PERIOD_LOG * 1000)) {
#ifdef CONS
    Serial.print("Counts: "); Serial.println(counts);
#endif
    cpm = (60000 * counts) / (millis() - startCountTime) ;
    counts = 0 ;
    startCountTime = millis() ;
    lastCountTime += PERIOD_LOG * 1000;
    display.clear();
    displayString("Radioactivity", 64, 0);
    displayInt(cpm, 64, 30);
    if (cpm >= 200) {
      if (active) {
        active = 0 ;
        display.clear();
        displayString("¡¡ALARM!!", 64, 0);
        displayInt(cpm, 64, 30);
#ifdef IFTT
        IFTTT(cpm);
#endif
      } ;
    }
    else if (cpm < 100)
    {
      active = 1 ;
    } ;
#ifdef CONS
    Serial.print("cpm: "); Serial.println(cpm);
#endif
    printStack();
  }

  if (millis() - lastEntryThingspeak > (PERIOD_THINKSPEAK * 1000)) {
#ifdef CONS
    Serial.print("Counts2: "); Serial.println(counts2);
#endif
    int averageCPH = (int)(((float)3600000 * (float)counts2) / (float)(millis() - startEntryThingspeak));
#ifdef CONS
    Serial.print("Average cph: "); Serial.println(averageCPH);
#endif
    postThingspeak(averageCPH);
    lastEntryThingspeak += PERIOD_THINKSPEAK * 1000;
    startEntryThingspeak = millis();
    counts2=0;
  } ;
  delay(50);
}

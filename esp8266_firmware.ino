#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secureConfig.h"
#include <WiFiClientSecure.h>
#include "ThingSpeak.h"

JsonDocument doc;

ESP8266WiFiMulti WiFiMulti;
WiFiClientSecure  client;

char ssid[] = WIFI_SSID;   // your network SSID (name) 
char pass[] = WIFI_PASSWORD;   // your network password

String message = "";
bool messageReady = false;
bool lightBlink = true;
bool lightOn = true;

const byte rxPin = 14; // d5
const byte txPin = 12; // d6

// Set up a new SoftwareSerial object
// probably going to switch to the hardware serial once everything is programmed
SoftwareSerial mySerial (rxPin, txPin);

const byte wifiLED = 5; // d1

unsigned long time_now = 0;
int period = 1000; // time between blinks

// Fingerprint check, make sure that the certificate has not expired.
const char * fingerprint = SECRET_SHA1_FINGERPRINT; // use SECRET_SHA1_FINGERPRINT for fingerprint check

bool sendData = true;
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = THINGSPEAK_API_WRITE;

int number = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(wifiLED, OUTPUT);
  // Set the baud rate for the SoftwareSerial object
  mySerial.begin(9600);
  // prepare wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
  // all this below does work, but im going to stick with the wifi multi setup i have going right now. May have to do that connect delay here later though. Also that client stuff.
  /*WiFi.begin(ssid, pass);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  if(fingerprint!=NULL) {
    client.setFingerprint(fingerprint);
  } else {
    client.setInsecure(); // To perform a simple SSL Encryption
  }
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  */
}

int providedNumber = 0;

void serialFlush(){
  while(mySerial.available() > 0) {
    char t = mySerial.read();
    Serial.println(t);
  }
}

void loop() {

  // put your main code here, to run repeatedly:
  while (mySerial.available()) { 
    message = mySerial.readString();
    serialFlush();
    messageReady = true;
  }
  if (messageReady == true) {
    messageReady = false;
    lightBlink = false;
    // this is where the data should be collected from serial and stored for upload every two minutes.
  }
  blinkLight(lightBlink); // built in led, blinking every second until this parameter is false
  wifiStatusLED(); // turn the wifi status led on if we are connected


  // flips between 0 and 1 every 60 seconds. (0 to min 1 = 0, 1 to 2 = 1, 2 to 3 = 0, etc) (sends data every two minutes, when 60000)
  // every (2 * 60000) seconds
  if (((millis() / 60000) % 2)) {
    if (sendData == true) {
      digitalWrite(wifiLED, HIGH);
      Serial.println((message));
      thingSpeakWriteREST(message);
      sendData = false;
    }
  } else {
    sendData = true;
  }
}

// called from loop, just blinks the light.
void blinkLight(bool keepBlinking) {
  if (lightBlink == true) {
    digitalWrite(LED_BUILTIN, (millis() / period) % 2);
  }
}

// write to thingspeak using a global api key and parameter data
void thingSpeakWriteREST(String data) {

  // prepare the json file then make it into a string
  prepareJSON(data);
  String payload;
  serializeJson(doc, payload);
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  HTTPClient https;
  String endpoint = "https://api.thingspeak.com/update.json";

  if (https.begin(*client, endpoint)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(payload);
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String serverUpdate = https.getString();
        Serial.println(serverUpdate);
        //success = true; // this should be passed in as a reference
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();

  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  
}

// total tokens will
void prepareJSON(String message) {
  doc["api_key"] = THINGSPEAK_API_WRITE;
  // tokenize the string, count the tokens, prepare the data.
  String totalTokens = getValue(message, ';', 0);
  int totalTokensCount = totalTokens.toInt();
  for (int i = 1; i <= totalTokensCount; i++) {
    String field = "field"+String(i);
    String dataValue = getValue(message, ';', i);
    doc[field] = dataValue.toDouble();
  }
  
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void wifiStatusLED() {
    if ((WiFiMulti.run() == WL_CONNECTED)) {
    digitalWrite(wifiLED, HIGH);
  } else {
    digitalWrite(wifiLED, LOW);
  }
}

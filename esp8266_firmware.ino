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
    String output = "output: " + message;
    Serial.println(output);
    lightBlink = false;
    // this is where the data should be collected from serial and stored for upload every two minutes.

    providedNumber = message.toInt();

  }
  blinkLight(lightBlink); // built in led, blinking every second until this parameter is false
  wifiStatusLED(); // turn the wifi status led on if we are connected


  // flips between 0 and 1 every 60 seconds. (0 to min 1 = 0, 1 to 2 = 1, 2 to 3 = 0, etc) (sends data every two minutes, when 60000)
  // every (2 * 60000) seconds
  if (((millis() / 60000) % 2)) {
    if (sendData == true) {
      // temporary check to overwrite the uploaded number
      if (providedNumber > 0) {
        digitalWrite(wifiLED, HIGH);
        number = providedNumber;
      }
      
      String message = "output: " + String(number) + " MILLIS: " + String(millis());
      Serial.println((message));
      thingSpeakWriteREST(number);
      numberCounter();
      sendData = false;
    }
  } else {
    sendData = true;
  }
}

void numberCounter() {
  number++;
  if(number > 99){
    number = 0;
  }
}

// called from loop, just blinks the light.
// 
void blinkLight(bool keepBlinking) {
  if (lightBlink == true) {
    /*if (millis() > time_now + period) {
      time_now = millis();
      digitalWrite(LED_BUILTIN, lightOn);
      lightOn = !lightOn; // switch the light
      Serial.println(lightOn);
   }  
  }*/
    digitalWrite(LED_BUILTIN, (millis() / period) % 2);
  }
}

// write to thingspeak using a global api key and parameter data
void thingSpeakWriteREST(int data) {

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

// https://www.mathworks.com/help/thingspeak/choose-between-rest-and-mqtt.html
void prepareJSON(int data) {
  doc["api_key"] = THINGSPEAK_API_WRITE;
  doc["field1"] = data;
}
/*
  import requests
    resp = requests.post('https://api.thingspeak.com/update.json', 
                         json={"api_key"="XXXXXXXXXXXXXXXX",
                         "field1"=73,
                         "field2"=66})

https://gist.github.com/vi3k6i5/5099e4fceeb4bff5eb0b35f7d5b7e298                         
*/

void wifiStatusLED() {
    if ((WiFiMulti.run() == WL_CONNECTED)) {
    digitalWrite(wifiLED, HIGH);
  } else {
    digitalWrite(wifiLED, LOW);
  }
}

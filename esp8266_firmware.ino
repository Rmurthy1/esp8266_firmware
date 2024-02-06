#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secureConfig.h"

ESP8266WiFiMulti WiFiMulti;


String message = "";
bool messageReady = false;
bool lightBlink = true;
bool lightOn = true;

const byte rxPin = 14; // d5
const byte txPin = 12; // d6

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);

const byte wifiLED = 5; // d1

unsigned long time_now = 0;
int period = 1000; // time between blinks

void setup() {
  // put your setup code here, to run once:
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
}

void loop() {
  // put your main code here, to run repeatedly:
  while (mySerial.available()) { 
    message = mySerial.readString();
    messageReady = true;
  }
  if (messageReady == true) {
    //do stuff
    messageReady = false;
    String output = "output: " + message;
    Serial.println(output);
    lightBlink = false;
    // this is where code should be uploaded. can it be done in a nonblocking way?
    
  }
  blinkLight(lightBlink);
  
// the loop function runs over and over again forever
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    digitalWrite(wifiLED, HIGH);
  } else {
    digitalWrite(wifiLED, LOW);
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



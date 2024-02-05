
String message = "";
bool messageReady = false;
bool lightBlink = true;

#include <SoftwareSerial.h>

const byte rxPin = 14; // d5
const byte txPin = 12; // d6

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
    
  // Set the baud rate for the SoftwareSerial object
  mySerial.begin(9600);
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
  }
  if (lightBlink == true) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(1000);  
  }


// the loop function runs over and over again forever

}



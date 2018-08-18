/*
 *  This sketch sends a message to a TCP server
 *
 */
#define redLed D3 //pin for LED
#define pinPOT A0 //pin for POT, Here we are using analog pin
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#define pin D2

ESP8266WiFiMulti WiFiMulti;

void setup() {
    Serial.begin(115200);
    pinMode(D2, OUTPUT);
    delay(10);

    // We start by connecting to a WiFi network
    WiFiMulti.addAP("NIGHTOWL", "VEgFXiRxs3");

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        Serial.println("LED");
        digitalWrite(pin,LOW);
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");

    delay(500);
}


void loop() {
int pot_val = analogRead(pinPOT); //taking values of POT
int led_val = map(pot_val, 0, 1023, 0, 255); //map command is described in Tutorial
analogWrite(redLed, led_val); //fading the led from the POT input
delay(50); // delay is given for stablilty purposes
Serial.println(led_val);

  digitalWrite(pin, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is active low on the ESP-01)
  delay(500);                      // Wait for a second
  digitalWrite(pin, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500);                      // Wait for two seconds (to demonstrate the active low LED)
}

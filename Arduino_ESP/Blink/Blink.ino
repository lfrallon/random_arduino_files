/*
 ESP8266 Blink by Simon Peter
 Blink the blue LED on the ESP-01 module
 This example code is in the public domain
 
 The blue LED on the ESP-01 module is connected to GPIO1 
 (which is also the TXD pin; so we cannot use Serial.print() at the same time)
 
 Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/
#define pin D2

void setup() {
  pinMode(pin,  OUTPUT);     // Initialize the LED_BUILTIN pin as an output
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(pin, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  delay(1000);                                  // but actually the LED is on; this is because 
                                    // it is active low on the ESP-01)
                      // Wait for a second
  digitalWrite(pin, LOW);  // Turn the LED off by making the voltage HIGH
  delay(1000);                      // Wait for two seconds (to demonstrate the active low LED)
}

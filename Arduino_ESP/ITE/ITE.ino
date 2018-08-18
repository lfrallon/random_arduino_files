

#define redLed D3 //pin for LED
#define pinPOT A0 //pin for POT, Here we are using analog pin
void setup() {
// put your setup code here, to run once:
pinMode(pinPOT, INPUT); //POT is declared as input
pinMode(redLed, OUTPUT); //LED is declared as output
Serial.begin(115200);
}
void loop() {
// put your main code here, to run repeatedly:
int pot_val = analogRead(pinPOT); //taking values of POT
int led_val = map(pot_val, 0, 1023, 0, 255); //map command is described in Tutorial
analogWrite(redLed, led_val); //fading the led from the POT input
delay(50); // delay is given for stablilty purposes
Serial.println(led_val);
}
 



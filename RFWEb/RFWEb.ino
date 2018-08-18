
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <SPI.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x96, 0xB4 };
byte ip[] = { 192,168,1,110 }; // Direction ip local
char server[]={"example.com"};
EthernetClient client;

LiquidCrystal lcd(9, 8, 7, 4, 3, 2);
byte serNum[5];
byte data[5];
// Choose two pins for SoftwareSerial
SoftwareSerial rSerial(2, 3); // RX, TX

//button
//const int lockbutton = 5;
const int openbutton = 4;
int rbutton = 0;

String Status = "";
int a = 0;
int b = 0;

//String Status;

//led
const int openled= 6;
const int closeled= 7;

// Make a servo object
Servo myServo;

// Pick a PWM pin to put the servo on
const int servoPin = 9;

// For SparkFun's tags, we will receive 16 bytes on every
// tag read, but throw four away. The 13th space will always
// be 0, since proper strings in Arduino end with 0

// These constants hold the total tag length (tagLen) and
// the length of the part we want to keep (idLen),
// plus the total number of tags we want to check against (kTags)
const int tagLen = 16;
const int idLen = 13;
const int kTags = 6;

// Put your known tags here!
char knownTags[kTags][idLen] = {
             "111111111111",
             "444444444444",
             "555555555555",
             "7A005B0FF8D6",
             "7E00200389D4",
             "7E001FFCA63B"

};

// Empty array to hold a freshly scanned tag
char newTag[idLen];

void setup() {
  // Starts the hardware and software serial ports
   Serial.begin(9600);
   rSerial.begin(9600);

   // Attaches the servo to the pin
   myServo.attach(servoPin);

   // Put servo in locked position
   myServo.write(0);
  pinMode(openled, OUTPUT);
  pinMode(closeled, OUTPUT);
  pinMode(openbutton, INPUT);
  Ethernet.begin(mac, ip); //Initiation ethernet shield
  delay(1000); // Waiting 1 second after initializing
  lcd.begin(16, 2);
  lcd.print(Ethernet.localIP());
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  Serial.begin(9600); // Serial communication initialization
  SPI.begin(); // SPI communication initialization
 // rfid.init(); // RFID module initialization
  lcd.setCursor(0,1);
  lcd.print("Waiting for Card");
  Serial.println("Waiting for Card");
}

void loop() {
rbutton = digitalRead(openbutton);
while(b == 0){
if(a == 1){
        Status = "Unlock";
        digitalWrite(closeled, LOW);
        digitalWrite(openled, HIGH);
      
      }else if(a == 0){
        Status = "Lock";
        digitalWrite(openled, LOW);
        digitalWrite(closeled, HIGH);
      }
Serial.println(Status);
b++;
}

 if(rbutton == HIGH){
     unLock();
     }
     else{
  
  // Counter for the newTag array
  int i = 0;
  // Variable to hold each byte read from the serial buffer
  int readByte;
  // Flag so we know when a tag is over
  boolean tag = false;

  // This makes sure the whole tag is in the serial buffer before
  // reading, the Arduino can read faster than the ID module can deliver!
  if (rSerial.available() == tagLen) {
    tag = true;
  }

  if (tag == true) {
    while (rSerial.available()) {
      // Take each byte out of the serial buffer, one at a time
      readByte = rSerial.read();

      /* This will skip the first byte (2, STX, start of text) and the last three,
      ASCII 13, CR/carriage return, ASCII 10, LF/linefeed, and ASCII 3, ETX/end of 
      text, leaving only the unique part of the tag string. It puts the byte into
      the first space in the array, then steps ahead one spot */
      if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
        newTag[i] = readByte;
        i++;
      }

      // If we see ASCII 3, ETX, the tag is over
      if (readByte == 3) {
        tag = false;
      }

    }
  }


  // don't do anything if the newTag array is full of zeroes
  if (strlen(newTag)== 0) {

    return;
  }

  else {
    int total = 0;

    for (int ct=0; ct < kTags; ct++){
        total += checkTag(newTag, knownTags[ct]);
    }

    // If newTag matched any of the tags
    // we checked against, total will be 1
    if (total > 0) {

      // Put the action of your choice here!
      if (client.connect(server, 80)) {
      
    if (tag){
    Serial.println("Connection Successfull");
    client.print("GET http://example.com/arduino.php?v=");
    client.print(data[0]);client.print(data[1]);client.print(data[2]);client.print(data[3]);client.print(data[4]);
    client.println(" HTTP/1.0"); client.println("User-Agent: Arduino 1.0");
    client.println(); client.stop(); lcd.clear();
    lcd.print("USER!");
    Serial.println("Hello USER!"); // print a message  
    delay(1000);
      // I'm going to rotate the servo to symbolize unlocking the lockbox
      lcd.setCursor(0,1);
   lcd.print("Access Granted!");
    Serial.println("Access Granted!... Welcome!"); // print a message
    delay(1000);
      Serial.print("| ACCESS GRANTED | ");
      Serial.print(newTag);
      Serial.print(" | ");
      unLock();
      Serial.println();
  lcd.clear();
  lcd.print("    Welcome!");
  lcd.setCursor(0,1);
  Serial.println("Waiting for Card");
  lcd.print("Waiting for Card");
  client.stop();
  delay(1000);  
  }
  
  /*
  // another cards analysis put many blocks like this as many user you have
  else if (USER2_card){
    Serial.println("Connection Successfull");
    client.print("GET http://example.com/arduino.php?v=");
    client.print(data[0]);client.print(data[1]);client.print(data[2]);client.print(data[3]);client.print(data[4]);
    client.println(" HTTP/1.0"); client.println("User-Agent: Arduino 1.0");
    client.println(); client.stop(); lcd.clear();
    lcd.print("USER2!");
    Serial.println("Hello USER2!"); // print a message  
    delay(1000);    
  }
  */
  
  else{ // if a card is not recognized
    
  }
 }
}

    else {
        // This prints out unknown cards so you can add them to your knownTags as needed
    lcd.clear();
    lcd.print("Card not");
    lcd.setCursor(0,1);
    lcd.print("recognized!");
    Serial.println("Card not recognized!"); // print a message
    delay(1000);
     Serial.print("| ACCESS DENIED  | ");
      Serial.print(newTag);
      Serial.print(" | ");
      myServo.write(30);
      Serial.println();
      a = 0;
      b = 0;
    Serial.println("Connection Unuccessfull");
    lcd.clear();
    lcd.print("Connection");
    lcd.setCursor(0,1);
    lcd.print("Unuccessful");
    delay(1000);
    lcd.clear();
    lcd.print("Try Again!");
    client.stop();
    delay(1000);
     
    }
  }

  // Once newTag has been checked, fill it with zeroes
  // to get ready for the next tag read
  for (int c=0; c < idLen; c++) {
    newTag[c] = 0;
  }

     }

}

void unLock()
{

//Unlocks the door by turning the servo
Status = "Unlock";
Serial.println(Status);
myServo.write(140);
digitalWrite(closeled, LOW);
digitalWrite(openled, HIGH);
delay(10000);
while(rSerial.available()>0){
  rSerial.read();
}
myServo.write(30);
a = 0;
b = 0;

}

// This function steps through both newTag and one of the known
// tags. If there is a mismatch anywhere in the tag, it will return 0,
// but if every character in the tag is the same, it returns 1
int checkTag(char nTag[], char oTag[]) {
    for (int i = 0; i < idLen; i++) {
      if (nTag[i] != oTag[i]) {
        return 0;
      }
    }
  return 1;
}

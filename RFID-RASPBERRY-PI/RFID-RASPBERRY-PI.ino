/*****************************
     RFID-powered lockbox

  This sketch will move a servo when
  a trusted tag is read with the
  ID-12/ID-20 RFID module

  Pinout for SparkFun RFID USB Reader
  Arduino ----- RFID module
  5V            VCC
  GND           GND
  D2            TX

  Pinout for SparkFun RFID Breakout Board
  Arduino ----- RFID module
  5V            VCC
  GND           GND
  D2            D0

  Connect the servo's power, ground, and
  signal pins to VCC, GND,
  and Arduino D9

  If using the breakout, you can also
  put an LED & 330 ohm resistor between
  the RFID module's READ pin and GND for
  a "card successfully read" indication

  by acavis, 3/31/2015

  Inspired by & partially adapted from
  http://bildr.org/2011/02/rfid-arduino/

******************************/

#include <SoftwareSerial.h>
#include <Servo.h>

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
const int openled = 6;
const int closeled = 7;

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

// Empty array to hold a freshly scanned tag
char newTag[idLen];

void setup() {
  // Starts the hardware and software serial ports
  Serial.begin(250000);
  rSerial.begin(9600);

  // Attaches the servo to the pin
  myServo.attach(servoPin);

  // Put servo in locked position
  myServo.write(0);
  pinMode(openled, OUTPUT);
  pinMode(closeled, OUTPUT);
  pinMode(openbutton, INPUT);
}

void loop() {
  rbutton = digitalRead(openbutton);
  while (b == 0) {
    if (a == 1) {
      digitalWrite(closeled, LOW);
      digitalWrite(openled, HIGH);

    } else if (a == 0) {
      digitalWrite(openled, LOW);
      digitalWrite(closeled, HIGH);
    }
    //Serial.println(Status);
    b++;
  }

  if (rbutton == HIGH) {
    unLock();
  }
  else {

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
        if (readByte != 2 && readByte != 13 && readByte != 10 && readByte != 3) {
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
    if (strlen(newTag) == 0) {

      return;
    }

    else {
      if (checkAccess(newTag)) {

        unLock();
      }

      else {
        // This prints out unknown cards so you can add them to your knownTags as needed
        lock();

      }
    }

    // Once newTag has been checked, fill it with zeroes
    // to get ready for the next tag read
    for (int c = 0; c < idLen; c++) {
      newTag[c] = 0;
    }

  }

}

bool checkAccess(String id){
  while(Serial.available()){
    //Wipe the Serial buffer to make sure there is no old data available
    Serial.read();
  }
  //Send the tag to the serial interface surrounded by ;starttag; and ;endtag;
  Serial.print(";starttag;");
  Serial.print(id);
  Serial.println(";endtag;");
  Serial.flush(); //wait for all data to be sent
  
  while(!Serial.available()){
    //wait for a response
    delay(10);
  }
  String response;
  while(Serial.available()>0){
    //start reading the response, one character at a time
    char inchar = Serial.read();
    response += inchar;
    if(response == "OK"){
      //if we found 'OK' in the response, grant access
      Serial.println(response);
      return true;
    }
    if(response == "DENIED"){
      //if we found 'DENIED' in the response, deny access
      Serial.println(response);
      return false;
    }
  }
  //We got data in the serial interface but it was neither OK nor DENIED, which we are expecting. To be safe, deny access.
  Serial.println("Something went wrong");
  return false;
}


  void lock() {
    while (rSerial.available() > 0) {
      rSerial.read();
    }
    myServo.write(35);
    Serial.println("Lock");
    a = 0;
    b = 0;
  }

  void unLock()
  {

    //Unlocks the door by turning the servo
    Serial.println("Unlock");
    myServo.write(140);
    digitalWrite(closeled, LOW);
    digitalWrite(openled, HIGH);
    delay(10000);
    lock();

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

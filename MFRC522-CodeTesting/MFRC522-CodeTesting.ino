// Sample Arduino Json Web Client
// Downloads and parse http://jsonplaceholder.typicode.com/users/1
//
// Copyright Benoit Blanchon 2014-2017
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
// If you like this project, please add a star!
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>
#include <MFRC522.h> //The RFID key library
#include <SoftwareSerial.h>
#include <HttpClient.h>
#include <Servo.h>

#define COMMON_ANODE

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

// Set Led Pins
#define yellowLed 5
#define blueLed 6

#define relay 4     // Set Relay Pin

#define RST_PIN         9           // Configurable, see typical pin layout above - This is for the Arduino Nano - For RFID
#define SS_PIN          8 //WE ARE USING 8 FOR RFID BECAUSE THE ETHERNET MODULE USES 10

  byte sector         = 0;
  byte blockAddr      = 0; ////////Access certain sector/blocks in the card, trailer block is the last block
  byte trailerBlock   = 1;

boolean match = false;          // initialize card match to false

uint16_t successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module

EthernetClient client;
Servo myServo;

const char* server = "rfid.test.nightowl.foundationu.com";  // server's address
String resource = "http://rfid.test.nightowl.foundationu.com/checkTag/";// http resource
const unsigned long BAUD_RATE = 9600;                 // serial connection speed
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response
const int servoPin = 3;
const int buttonPin = 2;

bool cardID[12];

//variables will change:
int buttonState = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
IPAddress ip(10, 99, 226, 117);
IPAddress gateway(10, 99, 226, 129);
IPAddress subnet(255, 255, 255, 128);

MFRC522::MIFARE_Key key; //Set key instance

signed long timeout; //TIMEOUT SO IT DOESN'T SIT THERE FOREVER

// ARDUINO entry point #1: runs once when you press reset or power the board

// Initialize Serial port
void initSerial() {
  Serial.begin(9600);

  myServo.attach(servoPin);
  //put servo in lock position
  myServo.write(0);
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(yellowLed, LED_ON);
  pinMode(buttonPin, INPUT);
  
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}

// Initialize Ethernet library
void initEthernet() {
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0xAE, 0x48};
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet");
    Serial.println("Connecting...");
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  Serial.println("Ethernet ready");
  delay(1000);
    for(int t = 255; t > 0; t--)
  {
    analogWrite(blueLed, t);           ////More of show but let at least a second between the SPI of the ethernet and RFID
    delay(10);
  }

}

// Open connection to the HTTP server
bool connect(const char* server) {
  Serial.print("Connect to ");
  Serial.println(server);

  bool ok = client.connect(server, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

String dump_byte_array(byte *buffer, byte bufferSize) {
          String out = "";
    for (byte i = 0; i < bufferSize; i++) {
        //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        //Serial.print(buffer[i], HEX);
        out += String(buffer[i] < 0x10 ? " 0" : " ") + String(buffer[i], HEX);
    }
    out.toUpperCase();
    out.replace(" ", "");
    return out;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* server, String resource) {
  Serial.print("GET ");
  Serial.println(resource);
  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.0");
  client.print("Host: ");
  client.println(server);
  Serial.println(server);
  client.println("Connection: close");
  client.println();

  return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Valid response!");
  return ok;
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

void sendData() {
     for (byte i = 0; i < 4; i++) {   // Prepare the key (used both as key A and as key B)
        key.keyByte[i] = readCard[i];        // using FFFFFFFFFFFF which is the default at chip delivery from the factory
        }
    //Serial.println(F("System is Ready:"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);     //Get key byte size
    timeout = 0;  
    delay(200);
}
//END DUMP_BYTE_ARRAY
////////////////////////////////////////  Access Granted    ///////////////////////////////////
void unlock() {
    digitalWrite(blueLed, LED_ON);   // Turn off blue LED
    delay(500);
    digitalWrite(blueLed, LED_OFF);
    delay(500);
    digitalWrite(yellowLed, LED_OFF);  // Turn off red LED
    Serial.println("Unlock");
    myServo.write(140);
    delay(10000); // Hold blue LED on for a second
    lock();            
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void lock() { 
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(yellowLed, LED_ON);   // Turn on red LED
   while (client.available() > 0) {
      client.read();
    }
    myServo.write(35);
    Serial.println("Lock");
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint16_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
      Serial.println(F("Scanned PICC's UID:"));
      digitalWrite(blueLed, LED_ON);    // Visualize Master Card need to be defined
      delay(1000);
      digitalWrite(blueLed, LED_OFF);
      delay(1000);
  for ( uint16_t i = 0; i < 8; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  sendData();
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(yellowLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(yellowLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(yellowLed, LED_ON);   // Make sure red LED is on
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint16_t number ) {
  uint16_t start = (number * 8 ) + 2;    // Figure out starting position
  for ( uint16_t i = 0; i < 8; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )      // Make sure there is something in the array first
    match = true;       // Assume they match at first
  for ( uint16_t k = 0; k < 8; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] )     // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {      // Check to see if if match is still true
    return true;      // Return true
  }
  else  {
    return false;       // Return false
  }
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint16_t findIDSLOT( byte find[] ) {
  uint16_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint16_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
      break;          // Stop looking we found it
    }
  }
}

bool checkHTTPAccess(String tagid) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  if (!connect(server)) {
    Serial.println("Connection to server failed");
    //check knowTags array, if in there return true
    return false;
  }
  if (sendRequest(server, resource+tagid) && skipResponseHeaders()) { 
    // Allocate a temporary memory pool
    Serial.println("Reading from the client");
    String resp="";
    while (!client.available()){
      delay(100);
    }
    while (client.available()) {
      char c = client.read();
      resp = resp + c;
    }
    Serial.print("Response:");
    Serial.print(resp);
    Serial.println(":EndResponse");
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(resp);
 
    if (!root.success()) {
      Serial.println("JSON parsing failed!");
      disconnect();
      return false;
    }
    Serial.println("JSON parsing success!");
    // Here were copy the strings we're interested in
    disconnect();
    //// add/remove tag from knowtags array
    return root["access"];
    // It's not mandatory to make a copy, you could just use the pointers
    // Since, they are pointing inside the "content" buffer, so you need to make
    // sure it's still in memory when you read the string\
  }  else{
    disconnect();
    return false;
  }
}
void setup() {
   //Arduino Pin Configuration
  pinMode(yellowLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(relay, OUTPUT);
  //Be careful how relay circuit behave on while resetting or power-cycling your Arduino
  digitalWrite(relay, HIGH);    // Make sure door is locked
  digitalWrite(yellowLed, LED_OFF);  // Make sure led is off
  digitalWrite(blueLed, LED_OFF); // Make sure led is off

  initSerial();
  initEthernet();
  //RFID INITIAL
  mfrc522.PCD_Init(); // Init MFRC522 card
  //If you set Antenna Gain to Max it will increase reading distance
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}

void loop() {
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(yellowLed, LED_ON);  // Make sure led is off
      digitalWrite(blueLed, LED_OFF); // Make sure led is off

      buttonState = digitalRead(buttonPin);

      // check if the pushbutton is pressed.
      //if it is, the buttonState is HIGH:
      if (buttonState == HIGH) {
        Serial.println("Exit Button is pressed.");
        unlock();
      }
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
    String tagid = dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  if (checkHTTPAccess(tagid)) {
     Serial.println("Access granted" );
     unlock();
  }
  else{
    Serial.println("Access denied");
    lock();
   } 
}
///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    //failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint16_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint16_t slot;       // Figure out the slot number of the card
    uint16_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint16_t looping;    // The number of times the loop repeats
    uint16_t j;
    uint16_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 8) + 2;
    looping = ((num - slot) * 8);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint16_t k = 0; k < 8; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    //successdelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint16_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint16_t start = ( num * 8 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint16_t j = 0; j < 8; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    //successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    //failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}
///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID( byte find[] ) {
  uint16_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint16_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
      break;  // Stop looking we found it
    }
    else {    // If not, return false
    }
  }
  return false;
}

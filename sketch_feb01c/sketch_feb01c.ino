
#include <SoftwareSerial.h>
#include <Servo.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h> 
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices

#define COMMON_ANODE

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

#define RST_PIN         9           // Configurable, see typical pin layout above - This is for the Arduino Nano - For RFID
#define SS_PIN          8 //WE ARE USING 8 FOR RFID BECAUSE THE ETHERNET MODULE USES 10

// Choose two pins for SoftwareSerial
SoftwareSerial rSerial(8, 12); // RX, TX

// Ethernet stuff
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xAE, 0x48 };
//char server[] = "192.168.88.241";
IPAddress server(192, 168, 88, 241);
char url[] = "/checkTag/";
IPAddress ip(192, 168, 88, 11);
EthernetClient client;
StaticJsonBuffer<200> jsonBuffer;

//button
//const int lockbutton = 5;
//const int openbutton = 4;
//int rbutton = 0;

String Status = "";
int a = 0;
int b = 0;


//String Status;

//led
int red = 7;
int blue = 5;
int green = 6;

//EthernetClient client;  //ETHERNET INSTANCE

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key; //Set key instance

signed long timeout; //TIMEOUT SO IT DOESN'T SIT THERE FOREVER

// Make a servo object
//Servo myServo;

// Pick a PWM pin to put the servo on
//const int servoPin = 9;

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
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, INPUT);
  //Reset();
  
  Serial.begin(9600);
  rSerial.begin(9600);

  // Attaches the servo to the pin
  //myServo.attach(servoPin);

  // Put servo in locked position
  //myServo.write(0);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  else{
    Serial.println("Setup connection using DHCP");
  }
  delay(1000); //time to setup connection
}

void loop(){
  
  if(!Serial.available()){
    delay(1000);
  }
  String id = "";
  while(Serial.available()){
    char s = Serial.read();
    Serial.print("Read char: ");
    Serial.println(s);
    id = id + s;
  }
  if(id!=""){
    bool access = checkAccessHTTP(id);
    Serial.print("Access: ");
    Serial.println(String(access));
  }
}

bool setupHTTPConnection(){
  //setup the ethernet client
  
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    return true;
  }
  else{
    Serial.println("connection failed");
    return false;
  }
}


bool checkAccessHTTP(String id){
  if(setupHTTPConnection()){
   // Make a HTTP request:
    client.print("GET ");
    client.print(String(url)+id);
    Serial.print("Url: ");
    Serial.println(String(url)+id);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  } else {
    return false;
  }
  String resp="";
  while (!client.available()){
    delay(100);
  }
  while (client.available()) {
    char c = client.read();
    resp = resp + c;
  }
  Serial.print("Response: ");
  Serial.println(resp);
  JsonObject& root = jsonBuffer.parseObject(resp);
  // if the server's disconnected, stop the client:

  if(root.success()){
    return root["access"];
  }
  else{
    Serial.println("Failed to parse JSON");
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
    //myServo.write(35);
    Serial.println("Lock");
    a = 0;
    b = 0;
  }

  void unLock()
  {

    //Unlocks the door by turning the servo
    Serial.println("Unlock");
    //myServo.write(140);
    digitalWrite(red, LOW);
    digitalWrite(blue, HIGH);
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

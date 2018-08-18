/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>  // Library for Mifare RC522 Devices

// Update these with values suitable for your network.
#define RST_PIN D8  // RST-PIN für RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  D4 // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2 

#define LED_PIN D1
#define LED_PIN2 D3
#define TAGSIZE 12

uint8_t successRead; //variable integer to keep if we hace successful read

byte readCard[8]; //Stores scanned ID
char temp[3];
char cardID[9];

const char* ssid = "NIGHTOWL";
const char* password = "VEgFXiRxs3";
const char* mqtt_server = "10.99.226.178";
const char* mqtt_username = "openhabian";
const char* mqtt_password = "5dc2deb5fe";
const char* mqtt_id = "loreto";
const char* publish_msg = "openhabian/loreto/test";
const char* publish_msg2 = "openhabian/loreto/mfrc522";
const char* subscribe_msg = "openhabian/event/firstled";
const char* subscribe2_msg = "openhabian/event/secondled";
uint8_t tags;

WiFiClient espClient;
PubSubClient client(espClient);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

MFRC522::MIFARE_Key key;

long lastMsg = 0;
char msg[50];
char button_value[50];
int value = 0;

void setup() {
  // Initialize the BUILTIN_LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); //Initialize MFRC522 hardware
  delay(250);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  Serial.println(F("-------------------"));
  Serial.println(F("Everything Ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  memset(button_value, 0, sizeof(button_value));
  strncpy(button_value, (char *)payload, length);

  if(strcmp(button_value, "true")==0) {
    digitalWrite(LED_PIN, HIGH);
  }
  else if(strcmp(button_value, "false")==0) {
    digitalWrite(LED_PIN, LOW);
  }
  // Switch on the LED if an 1 was received as first character
  else if ((char)payload[0] == '1') {
    digitalWrite(LED_PIN2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LED_PIN2, LOW);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(publish_msg, "hello world");
      client.publish(publish_msg2, "Scanned Card");
      // ... and resubscribe
      client.subscribe(subscribe_msg);
      client.subscribe(subscribe2_msg);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
do {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
    successRead = getID();
    
    long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publish_msg, msg);
    }
  }
  while (!successRead);
}

void cycleLeds() {
  digitalWrite(LED_PIN2, LOW);  // Make sure red LED is off
  digitalWrite(LED_PIN, HIGH);   // Make sure green LED is on
  delay(200);
  digitalWrite(LED_PIN2, LOW);  // Make sure red LED is off
  digitalWrite(LED_PIN, HIGH);   // Make sure green LED is on
  delay(200);
  digitalWrite(LED_PIN2, LOW);  // Make sure red LED is off
  digitalWrite(LED_PIN, HIGH);   // Make sure green LED is on
  delay(200);

}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
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
  int j =0;
  for ( uint8_t i = 0; i < mfrc522.uid.size; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
    sprintf(temp, "%02X", readCard[i]);
    cardID[j] = temp[0];
    cardID[j+1] = temp[1];
    j=j+2;
  }
  Serial.println("");
  Serial.println("Publishing: ");
  client.publish(publish_msg2, cardID);
  Serial.println(cardID);
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}


//* MFRC522 esp8266
//* -----------------------------------------------------------------------------------------------------------
//*RST GPIO15
//*SDA(SS) GPIO2 
//*MOSI GPIO13
//*MISO GPIO12
//*SCK GPIO14
//*GND GND
//*3,3V 3,3V
//* -----------------------------------------------------------------------------------------------------------
//With this small example you should be able to connect to your WiFi Network and read a Rfid-Card (Output of the UID of the Card check on Serial):

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define RST_PIN  15 // RST-PIN für RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  2 // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2 

#define redLed 3

boolean match = false;
boolean programMode = false;
boolean replaceMaster = false;

uint8_t successRead;

byte storedCard[4];
byte readCard[4];
byte masterCard[4];

const char *ssid =  "NIGHTOWL"; // change according to your Network - cannot be longer than 32 characters!
const char *pass =  "VEgFXiRxs3"; // change according to your Network

const char* host = "10.99.226.192";
String path      = "/esp8266/light.json";

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {

pinMode(redLed, OUTPUT);

String Statuses[] = { "WL_IDLE_STATUS=0", "WL_NO_SSID_AVAIL=1", "WL_SCAN_COMPLETED=2", "WL_CONNECTED=3", "WL_CONNECT_FAILED=4", "WL_CONNECTION_LOST=5", "WL_DISCONNECTED=6"};

Serial.begin(115200); // Initialize serial communications
delay(10);

Serial.println(F("Booting...."));
Serial.print("Connecting to ");
Serial.println(ssid);

SPI.begin();  // Init SPI bus
mfrc522.PCD_Init(); // Init MFRC522

WiFi.begin(ssid, pass);
int wifi_ctr = 0;
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
Serial.println("WiFi connected");  
Serial.println("IP address: " + WiFi.localIP());

Serial.println(F("Ready!"));
Serial.println(F("======================================================")); 
Serial.println(F("Scan for Card and print UID:"));
}

void loop() {
  //ESP.wdtFeed();
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 8888;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");

  delay(500); // wait for server to respond

  // read response
  String section="header";
  while(client.available()){
    String line = client.readStringUntil('\r');
    // Serial.print(line);
    // we’ll parse the HTML body here
    if (section=="header") { // headers..
      Serial.print(".");
      if (line=="\n") { // skips the empty space at the beginning 
        section="json";
      }
    }
    else if (section=="json") {  // print the good stuff
      section="ignore";
      String result = line.substring(1);

      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }

      // Make the decision to turn off or on the LED
      if (strcmp(json_parsed["light"], "on") == 0) {
        digitalWrite(pin, HIGH); 
        Serial.println("LED ON");
      }
      else {
        digitalWrite(pin, LOW);
        Serial.println("led off");
      }
    }
  }
  Serial.print("closing connection. "); 
// Look for new cards
if ( ! mfrc522.PICC_IsNewCardPresent()) {
delay(50);
return;
}
// Select one of the cards
if ( ! mfrc522.PICC_ReadCardSerial()) {
delay(50);
return;
}
// Dump debug info about the card; PICC_HaltA() is automatically called
mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
Serial.println(F("======================================================"));
}

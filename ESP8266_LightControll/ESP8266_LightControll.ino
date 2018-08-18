#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid     = "NIGHTOWL";
const char* password = "VEgFXiRxs3";

const char* host     = "10.99.226.192"; // Your domain, no slashes (ex: www.nyl.io)  
String path          = "/esp8266/light.json"; // path to light.json beginning with slash (ex: /esptest/light.json)  
const int pin        = 2;

void setup() {  
  pinMode(pin, OUTPUT); 
  pinMode(pin, HIGH);

  String Statuses[] =  { "WL_IDLE_STATUS=0", "WL_NO_SSID_AVAIL=1", "WL_SCAN_COMPLETED=2", "WL_CONNECTED=3", "WL_CONNECT_FAILED=4", "WL_CONNECTION_LOST=5", "WL_DISCONNECTED=6"};
  
  Serial.begin(115200);

//  ESP.wdtDisable();
//  ESP.wdtEnable(WDTO_8S);

  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
                                      
  WiFi.begin(ssid, password);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: " + WiFi.localIP());
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
}

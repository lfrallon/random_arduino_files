#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define ledRed D3
#define ledGreen D2

// Update these with values suitable for your network.

const char* ssid = "NIGHTOWL";
const char* password = "VEgFXiRxs3";
const char* mqtt_username = "openhabian";
const char* mqtt_password = "5dc2deb5fe";
const char* mqtt_id = "botch";
const char* mqtt_server = "10.99.226.178";
char button_value[50];
const char* pub = "openhabian/message";
const char* sub = "openhabian/ledonoff";
const char* sub2 = "openhabian/ledbutton";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
 strncpy(button_value, (char *)payload, length); //Set the variable to the data of the new payload we received in our message

  if(strcmp(button_value, "true")==0){ //Compare the button value to what we except, if they are identical, change the LED_PIN.
    digitalWrite(ledGreen, HIGH);
  }
  else if(strcmp(button_value, "false")==0){
    digitalWrite(ledGreen, LOW);
  }

  
  // Switch on the LED if an 1 was received as first character
  else if ((char)payload[0] == '0') {
    digitalWrite(ledRed, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(ledRed, HIGH);  // Turn the LED off by making the voltage HIGH
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
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(sub);
      client.subscribe(sub2);
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

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(pub, msg);
  }
}

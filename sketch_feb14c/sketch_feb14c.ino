#include <Ethernet.h>

EthernetClient client;

IPAddress ip(10, 99, 226, 200);
IPAddress gateway(10, 99, 226, 129);
IPAddress subnet(255, 255, 255, 128);
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;
const char* server = "rfid.test.nightowl.foundationu.com";  // server's address
const char resource[] = "http://rfid.test.nightowl.foundationu.com/checkTag/25";
char response[50];

void initEthernet() {
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0xAE, 0x48};
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet");
    Serial.println("Connecting...");
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  Serial.println("Ethernet ready");
  delay(1000);
}

bool connect(const char* server) {
  Serial.print("Connect to ");
  Serial.println(server);

  bool ok = client.connect(server, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

bool sendRequest(const char* server, char *url) {
  Serial.print("GET ");
  Serial.println(url);
  client.print("GET ");
  client.print(url);
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
    return ok;
  }
  Serial.println("Valid response!");
  return ok;
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  initEthernet();
  connect(server);
  if(sendRequest(server, resource) && skipResponseHeaders()){
    Serial.println("Successfully connected");
  }
  while(1);
}

void loop() {
  // put your main code here, to run repeatedly:

}

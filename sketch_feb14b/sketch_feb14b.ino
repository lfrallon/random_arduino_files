#include <SPI.h>
#include <Ethernet.h>

#define TAGSIZE 12
#define yellowLed 5
#define blueLed 6

EthernetClient client;
const unsigned long BAUD_RATE = 9600;
const char* server = "rfid.test.nightowl.foundationu.com";  // server's address
const char resource[] = "http://rfid.test.nightowl.foundationu.com/checkTag/";// http resource
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
char url[sizeof(resource)+TAGSIZE];

void initSerial() {
  Serial.begin(BAUD_RATE);

  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}

bool initEthernet() {
  Serial.println("Initializing ethernet");
  //byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0xAE, 0x48};
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  int ok = Ethernet.begin(mac);
  if (ok == 0) {
    Serial.println("Failed to configure Ethernet");
  //  Serial.println("Connecting...");
  //  Ethernet.begin(mac, ip, gateway, subnet);
    return false;
  }
  Serial.println("Ethernet ready");
  delay(1000);
  for(int t = 255; t > 0; t--)
  {
    analogWrite(blueLed, t); //More of show but let at least a second between the SPI of the ethernet and RFID
    delay(10);
  }
  return true;

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

bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
    return ok;
  }
  //Serial.println("Valid response!");
  return ok;
}

void setup() {
  // put your setup code here, to run once:
  initSerial();
  initEthernet();
  connect(server);
  if(sendRequest(server, "rfid.nightowl.foundationu.com/checkTag/25") && skipResponseHeaders()){
    Serial.println("Got a valid response");
  }
  while (!client.available()){
    delay(100);
  }
  char response[50];
  int i=0;
  while (client.available() > 0) {
    char c = client.read();
    if(i<50){
      response[i]=c;
      i++;
    }
  }
  Serial.println(response);

}

void loop() {
  // put your main code here, to run repeatedly:

}


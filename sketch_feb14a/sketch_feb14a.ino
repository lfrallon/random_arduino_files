#include <SPI.h>

#include <Ethernet.h>

EthernetClient client;
const char server[] = "rfid.test.nightowl.foundationu.com";
const byte server_ip[] = {10, 99, 3, 200};
const char url[] = "rfid.test.nightowl.foundationu.com/checkTag/346AF4A50000";
const byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0xAE, 0x48};

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
void(* resetFunc) (void) = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //resetFunc();
  
  if (Ethernet.begin(mac) == 0){
    Serial.println("Failed to configure ethernet.");
  }
  else{
    Serial.println("Configured DHCP correctly");
  }
  printIPAddress();
  delay(2000);
  Serial.print("connecting to server ");
  Serial.println(server);
  int ok = client.connect(server, 80);
  while(ok!=1){
    Serial.print("Connection: ");
    Serial.println(ok);
    Serial.println("Resetting");
    client.stop();
    delay(2000);
    resetFunc();
  }
  
  Serial.println(ok ? "Connected" : "Connection Failed!");
  client.print("GET");
  Serial.print("url: ");
  Serial.println(url);
  client.print(url);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

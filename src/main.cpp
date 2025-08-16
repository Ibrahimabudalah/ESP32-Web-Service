#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets_autogen.h"

// initialize and start a server on port 80 - TCP
WiFiServer TCPServer(80);
WiFiUDP UDPServer;
WiFiClient client;
String data = "Welcome client\n";
String clientData = " ";
int packetSize = 0;
char incomingPacket[256];
const char *UDPData = "Message received\n";
boolean greetingPrinted = false;
WebServer webServer(80);

void setup() {

  // initialize serial communication
  Serial.begin(115200);
  // initialize wifi by passing in the ssid and password -- .env file
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // check if it's not connected
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }

  // print the successful connection and board IP address
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

  // start server
  TCPServer.begin();

  UDPServer.begin(5000);
}

void loop() {
  // check if no client - assign newly connected client to client
  if(!client || !client.connected()){
    WiFiClient newClient = TCPServer.available();

    if(newClient){
      client = newClient;

      Serial.println("Connected to client!");
      Serial.println(client.remoteIP());

      // send data variable to client
      client.print(data);
    } 
  }

  //if there is a client connected read the sent data on server
   if(client.available() > 0){
    clientData = client.readString();
    client.readStringUntil('\n');
    Serial.println(clientData);
  }


  // checks if there is an available packed and returns its size
  packetSize = UDPServer.parsePacket();
  
  if(packetSize > 0){
    
    // print the greeting message only this first time the client sends a packet then set it to false
    if(!greetingPrinted){
      Serial.printf("Client connected through UDP on port %d\n", UDPServer.remotePort());
    }
    greetingPrinted = true;

    // reads from buffer to buffer - 1 and return the number of bytes read
    int len = UDPServer.read(incomingPacket, 255);
    // end the string with null operator and print it
    incomingPacket[len] = '\0';
    Serial.println(incomingPacket);

    // begin connection to write data from server to client
    UDPServer.beginPacket(UDPServer.remoteIP(), UDPServer.remotePort());
    // write the data from pointer arr + string length
    UDPServer.write((const uint8_t*)UDPData, strlen(UDPData));
    //end after succesfully sending the packet
    UDPServer.endPacket();
  }
  delay(10);
}
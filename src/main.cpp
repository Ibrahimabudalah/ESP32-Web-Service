#include <Arduino.h>
#include <WiFi.h>
#include "secrets_autogen.h"

// initialize and start a server on port 80 - TCP
WiFiServer server(80);
WiFiClient client;
String data = "Welcome client\n";
String clientData = " ";

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
  server.begin();
}

void loop() {

  // check if no client - assign newly connected client to client
  if(!client || !client.connected()){
    WiFiClient newClient = server.available();

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
}
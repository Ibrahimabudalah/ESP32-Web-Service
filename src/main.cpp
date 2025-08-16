#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets_autogen.h"

// GPIO 2
int LED = 2;
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
// initialize and start a server on port 8080 - HTTP
WebServer webServer(8080);

const char homePage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 LED Control</title>
</head>
<body style="margin:0; padding:0; font-family:Arial, sans-serif; background:linear-gradient(135deg, #74ebd5, #ACB6E5); height:100vh; display:flex; justify-content:center; align-items:center;">

  <div style="text-align:center; background:#ffffffcc; padding:40px; border-radius:20px; box-shadow:0 8px 20px rgba(0,0,0,0.2); max-width:90%;">
    <h1 style="margin-bottom:30px; color:#333; font-size:clamp(24px, 5vw, 36px);">ESP32 LED Control</h1>
    <a href="/on">
      <button style="padding:15px 40px; margin:10px; font-size:clamp(16px, 4vw, 18px); border:none; border-radius:10px; background:#4CAF50; color:white; cursor:pointer; box-shadow:0 4px 10px rgba(0,0,0,0.2); transition:0.3s;">
        Turn ON
      </button>
    </a>
    <a href="/off">
      <button style="padding:15px 40px; margin:10px; font-size:clamp(16px, 4vw, 18px); border:none; border-radius:10px; background:#F44336; color:white; cursor:pointer; box-shadow:0 4px 10px rgba(0,0,0,0.2); transition:0.3s;">
        Turn OFF
      </button>
    </a>
  </div>

</body>
</html>
)rawliteral";


const char notFoundPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>404 Not Found</title>
</head>
<body style="margin:0; padding:0; font-family:Arial, sans-serif; background:linear-gradient(135deg, #ff9a9e, #fad0c4); height:100vh; display:flex; justify-content:center; align-items:center;">

  <div style="text-align:center; background:#ffffffcc; padding:40px; border-radius:20px; box-shadow:0 8px 20px rgba(0,0,0,0.2); max-width:90%;">
    <h1 style="margin-bottom:20px; color:#333; font-size:clamp(32px, 8vw, 48px);">404</h1>
    <h2 style="margin-bottom:20px; color:#555; font-size:clamp(20px, 5vw, 28px);">Page Not Found</h2>
    <p style="color:#666; margin-bottom:30px; font-size:clamp(14px, 4vw, 18px);">
      Sorry, the page you are looking for doesn’t exist.
    </p>
    <a href="/" style="padding:12px 30px; font-size:16px; border:none; border-radius:8px; background:#4CAF50; color:white; text-decoration:none; box-shadow:0 4px 10px rgba(0,0,0,0.2); transition:0.3s;">
      Go Home
    </a>
  </div>

</body>
</html>
)rawliteral";

// method to send the home page html to server
void handle_OnConnect(){
  webServer.send(200, "text/html", homePage);
}

// method to send the 404 page to server
void handleNotFound(){
  webServer.send(404, "text/html", notFoundPage);
}

// method to turn led on or off based on url path
void handleLED(){
  pinMode(LED, OUTPUT);
  String urlPath = webServer.uri();

  if(urlPath == "/on"){
    digitalWrite(LED, 1);
  }
  else if (urlPath == "/off"){
    digitalWrite(LED, 0);
  }
  webServer.send(200, "text/html", homePage);
}

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

  // start servers
  TCPServer.begin();

  UDPServer.begin(5000);
  
  webServer.on("/", handle_OnConnect);
  webServer.on("/on", handleLED);
  webServer.on("/off", handleLED);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP Server Started!");
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

  //listen for incoming reqs for HTTP server
  webServer.handleClient();
  delay(10);
}


#include <Arduino.h>
#include <WiFi.h>
#include "secrets_autogen.h"


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
}

void loop() {
  // put your main code here, to run repeatedly:
}
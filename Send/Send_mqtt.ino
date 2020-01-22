#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <ArduinoJson.h>
#include <MqttConnector.h>
#include <Wire.h>
#include <SPI.h>

#include "init_mqtt.h"
#include "_publish.h"
#include "_receive.h"
#include "_config.h"

MqttConnector *mqtt;

int relayPin = 15;
int relayPinState = HIGH;
int LED_PIN = 2;


char myName[40];

//////////////////////////////////////////////////////////////////////////
#define RXD2 17
#define TXD2 18
//////////////////////////////////////////////////////////////////////////

void init_hardware()
{
  pinMode(relayPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(relayPin, relayPinState);;
  // serial port initialization
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));
  pinMode(14, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);

  Serial.println();
  Serial.println("Starting...");
}

void init_wifi() {
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(50);
  const char* ssid =  WIFI_SSID.c_str();
  const char* pass =  WIFI_PASSWORD.c_str();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", ssid, pass);
    delay(300);
  }
  Serial.println("WiFi Connected.");
  digitalWrite(2, HIGH);
}

void setup()
{
  init_hardware();
  init_wifi();
  init_mqtt();
}

void loop()
{
  mqtt->loop();
  convertINPUT();
  digitalWrite(14, LOW);
}

void convertINPUT () {
  char readINPUT;
  if (Serial2.available() > 0) {
    readINPUT = Serial2.read();
    //Serial.println(readINPUT);

    if (readINPUT == 'V') {
      //Serial.println("VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV");
      checkINPUT = 1;
      Input = " ";
    }
    if (readINPUT == 'U') {
      //Serial.println("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");
      checkINPUT = 2;
      Input = " ";
    }
    if (readINPUT == '.') {
      //Serial.println("...........................................");
      Input += readINPUT;
    }
    if (readINPUT >= '0' && readINPUT <= '9') {
      //Serial.println("NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER");
      Input += readINPUT;
    }
  }

  if (checkINPUT == 1) {
    voltage = Input.toFloat();
  }
  else if (checkINPUT == 2) {
    distance = Input.toInt();
  }
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    Serial.print("Read voltage : ");
    Serial.print(voltage);
    Serial.println(" v");
    Serial.print("Read distance : ");
    Serial.print(distance);
    Serial.println(" cm");
  }
}

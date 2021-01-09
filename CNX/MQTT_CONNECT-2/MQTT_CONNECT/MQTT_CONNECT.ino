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
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
int zxcv = 1;
#include "_receive.h"
#include "_config.h"

MqttConnector *mqtt;

int relayPin = 15;
int relayPinState = HIGH;
int LED_PIN = 2;

int PLState, distance;
float voltage;

char myName[40];

//////////////////////////////////////////////////////////////////////////
#define RXD2 17
#define TXD2 18
//////////////////////////////////////////////////////////////////////////

void init_hardware()
{
  //Interrupt Pin
  pinMode(25, INPUT);
  attachInterrupt(digitalPinToInterrupt(25), ShowState, FALLING);

  pinMode(relayPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(relayPin, relayPinState);;

  // serial port initialization
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);
  pinMode(23, INPUT);

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

void convertINPUT () {
  String readINPUT;
  unsigned int INDEX;
  if (Serial2.available() > 0) {
    if (Serial2.read() == 'V') {
      readINPUT = Serial2.readString();
      //      Serial.println(readINPUT);
      INDEX = readINPUT.indexOf('>');
      //      Serial.println(INDEX);
    }
  }
  String subdonevol = readINPUT.substring(0, 4);
  voltage = subdonevol.toFloat();
  if (INDEX == 8) {
    String subdonedis = readINPUT.substring(5, 6);
    distance = subdonedis.toInt();
    String subdonestate = readINPUT.substring(7);
    PLState = subdonestate.toInt();
  }
  else if (INDEX == 9) {
    String subdonedis = readINPUT.substring(5, 7);
    distance = subdonedis.toInt();
    String subdonestate = readINPUT.substring(8);
    PLState = subdonestate.toInt();
  }
  else if (INDEX == 10) {
    String subdonedis = readINPUT.substring(5, 8);
    distance = subdonedis.toInt();
    String subdonestate = readINPUT.substring(9);
    PLState = subdonestate.toInt();
  }

  if (voltage > 0.00) {
    Serial.print("Your Voltage : ");
    Serial.print(voltage);
    Serial.println(" v");
    Serial.print("Your Distance : ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.print("Parking lock State : ");
    Serial.println(PLState);
  }
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

  unsigned long currentMillis2 = millis();
  if (((currentMillis2 - previousMillis2) >= 10000) && zxcv == 0) {
    zxcv = 1;
    attachInterrupt(digitalPinToInterrupt(25), ShowState, FALLING);
  }
}

#include <Arduino.h>
#include <MqttConnector.h>

extern MqttConnector* mqtt;

extern String MQTT_CLIENT_ID;
extern String MQTT_PREFIX;

extern int relayPin;
extern int relayPinState;
extern char myName[];

unsigned long previousMillisloop = 0;

void ShowState () {
  Serial.println("Interrupt triggered");
  digitalWrite(32, HIGH);
  detachInterrupt(25);
}

void register_receive_hooks() {
  
  mqtt->on_subscribe([&](MQTT::Subscribe * sub) -> void {
    Serial.printf("myName = %s \r\n", myName);
    sub->add_topic(MQTT_PREFIX + myName + "/$/+");
    sub->add_topic(MQTT_PREFIX + MQTT_CLIENT_ID + "/$/+");
  });

  mqtt->on_before_message_arrived_once([&](void) { });

  mqtt->on_message([&](const MQTT::Publish & pub) { });

  mqtt->on_after_message_arrived([&](String topic, String cmd, String payload) {
//    Serial.printf("topic: %s\r\n", topic.c_str());
//    Serial.printf("cmd: %s\r\n", cmd.c_str());
    Serial.printf("payload: %s\r\n", payload.c_str());
    
    if (cmd == "$/command") {
      if (payload == "W") {
        digitalWrite(14, HIGH);
        delay(100);
      }
      else if (payload == "S") {
        Serial2.println('S');
        digitalWrite(32, LOW);
//        digitalWrite(33, LOW);
      }
      else if (payload == "UP") {
        detachInterrupt(25);
        digitalWrite(13, HIGH);
        digitalWrite(32, LOW);
//        digitalWrite(33, LOW);
        zxcv = 0;
        previousMillis2 = millis();
        digitalWrite(14, HIGH);
        delay(100);
      }
      else if (payload == "DOWN") {
        detachInterrupt(25);
        digitalWrite(13, LOW);
        digitalWrite(32, LOW);
//        digitalWrite(33, LOW);
        zxcv = 0;
        previousMillis2 = millis();
        digitalWrite(14, HIGH);
        delay(100);
      }
    }
    else if (cmd == "$/reboot") {
      ESP.restart();
    }
    else {
      // another message.
    }
    digitalWrite(14, LOW);
  });
}

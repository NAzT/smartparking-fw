#include <Arduino.h>
#include <MqttConnector.h>

extern MqttConnector* mqtt;

extern String MQTT_CLIENT_ID;
extern String MQTT_PREFIX;

extern int relayPin;
extern int relayPinState;
extern char myName[];

extern int LED_PIN;

extern float voltage = 0;
extern char DATA;
extern String convert = " ", Input = " ";
extern int checkINPUT = 0, distance = 0;
extern unsigned long previousMillis = 0;

void register_receive_hooks() {
  mqtt->on_subscribe([&](MQTT::Subscribe * sub) -> void {
    Serial.printf("myName = %s \r\n", myName);
    sub->add_topic(MQTT_PREFIX + myName + "/$/+");
    sub->add_topic(MQTT_PREFIX + MQTT_CLIENT_ID + "/$/+");
  });

  mqtt->on_before_message_arrived_once([&](void) { });

  mqtt->on_message([&](const MQTT::Publish & pub) { });

  mqtt->on_after_message_arrived([&](String topic, String cmd, String payload) {
    Serial.printf("topic: %s\r\n", topic.c_str());
    Serial.printf("cmd: %s\r\n", cmd.c_str());
    Serial.printf("payload: %s\r\n", payload.c_str());
    digitalWrite(14, HIGH);
    if (cmd == "$/command") {
      Serial.println("now we in CMD");
      if (payload == "W") {
        Serial.println("Just wake up");
        Input = " ";
        checkINPUT = 0;
      }
      if (payload == "V") {
        Serial.println("now we in read Voltage");
        Serial2.println('V');
        Input = " ";
        checkINPUT = 0;
      }
      else if (payload == "U") {
        Serial.println("now we in read Distance");
        Serial2.println('U');
        Input = " ";
        checkINPUT = 0;
      }
      else if (payload == "S") {
        Serial.println("now we in go to Sleep");
        Serial2.println('S');
        
      }
      else if (payload == "UP") {
        digitalWrite(13, HIGH);
        delay(200);
      }
      else if (payload == "DOWN") {
        digitalWrite(13, LOW);
        delay(200);
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

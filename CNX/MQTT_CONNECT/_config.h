#include <Arduino.h> 

/* BOARD INFO */
String DEVICE_NAME      = "DEVICE-TEST"; 

/* WIFI INFO */ 
String WIFI_SSID        = "CMMC_3BB_2.4G";
String WIFI_PASSWORD    = "zxc12345";
/* MQTT INFO */ 
String MQTT_HOST        = "broker.hivemq.com";
String MQTT_USERNAME    = "";
String MQTT_PASSWORD    = "";
String MQTT_CLIENT_ID   = "";
String MQTT_PREFIX      = "PARK/";

int    MQTT_PORT        = 1883;
// 1L = 1ms , 1000L = 1sec
int PUBLISH_EVERY       = 1000L * 1;
int MQTT_CONNECT_TIMEOUT= 10; 

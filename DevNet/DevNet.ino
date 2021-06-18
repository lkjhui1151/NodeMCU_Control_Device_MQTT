
#include "M5Atom.h"
#include <WiFiManager.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <Esp32WifiManager.h>
#include <WiFi.h>
WiFiManager wm;
//Create a wifi manager
WifiManager manager;
String sent;
String topic_str, payload_str;
//// WiFi Setting
//#define WIFI_STA_NAME "CECRRU-WIFI"
//#define WIFI_STA_PASS NULL

// MQTT Setting
#define MQTT_SERVER "172.16.20.123"
#define MQTT_PORT 1883
#define MQTT_USERNAME "administrator"
#define MQTT_PASSWORD "P@ssw0rd@DevNet"
#define MQTT_NAME "DevNet"
WiFiClient client;         // Get WiFiClient for connecting to TCP.
PubSubClient mqtt(client); // Send the object from WiFiClient to the active PubSubClient library named mqtt.
// LED
int LED = 13;

void setup()
{
  // Connect to Network Device
  M5.begin();
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // 16 RXD,17 TXD
  // Serial Baud Rate
  Serial.begin(115200);
  //  Setup_WiFi();
  manager.setupScan();
  //  Serial.print(WiFi.macAddress());
  wm.autoConnect("30:AE:A4:03:3C:CC");
  // Connect to MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
  // LED CHECK STATUS WIFI
  pinMode(LED, OUTPUT);
}

void loop()
{
  mqtt_loop();
  if (Serial2.available())
  {
    sent = Serial2.readString();
    comparison(sent);
    mqtt_pub(sent);
  }
  if (Serial.available())
  {
    Serial2.write(Serial.read());
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
  }
}

void Setup_WiFi()
{
  // read the serial port for new passwords and maintain connections
  manager.loop();
  if (manager.getState() == Connected) {
    // use the Wifi Stack now connected
  }
  else {
    //reset saved settings
    wm.resetSettings();
  }
}

void mqtt_loop()
{
  if (mqtt.connected() == false) {
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print("connected");
      mqtt.subscribe("/30:AE:A4:03:3C:CC/DevNet/String_Serial/Sub");
    }
    else {
      Serial.println("failed");
      delay(5000);
    }
  }
  else {
    mqtt.loop();
  }
}

// MQTT calback
void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = '\0';
  topic_str = topic;
  payload_str = (char *)payload;
  //    Serial.println(payload_str);
  if (payload_str != NULL) {
    Serial2.write(13);
    Serial2.write(payload_str.c_str());
    Serial2.write(13);
  }
}
String mqtt_pub(String mqtt_input)
{
  delay(1000);
  mqtt.publish("/30:AE:A4:03:3C:CC/DevNet/String_Serial/Pub", mqtt_input.c_str());
}

// String comparion to Network Device
String comparison(String data_input)
{
  Serial.println(data_input);
  if (data_input.indexOf("Would you like to enter the initial configuration dialog? [yes/no]:") > -1 | data_input.indexOf("System configuration has been modified. Save? [yes/no]:") > -1)
  {
    Serial2.write("no");
    Serial2.write(13);
  }
  else if (data_input.indexOf("Press RETURN to get started!") > -1 | data_input.indexOf("Press RETURN to get started.") > -1 | data_input.indexOf(">") > -1)
  {
    Serial2.write(13);
    Serial2.write("enable");
    Serial2.write(13);
  }
  else if (data_input.indexOf("More") > -1 | data_input.indexOf("!") > -1 | data_input.indexOf("export@cisco.com.") > -1) {
    Serial2.write(32);
  }
  else if (data_input.indexOf("Erasing the nvram filesystem will remove all configuration files! Continue? [confirm]") > -1 | data_input.indexOf("Proceed with reload? [confirm]") > -1) {
    Serial2.write(13);
  }
  else if (data_input.indexOf("Password:") > -1) {
    Serial2.write("P@ssw0rd@iot");
    Serial2.write(13);
  }
}

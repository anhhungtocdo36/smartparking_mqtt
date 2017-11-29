#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "SSD1306.h"

// Update these with values suitable for your network.
#define ssid "SolarBKSES Guest"
#define password ""
#define mqtt_server "m14.cloudmqtt.com"
#define mqtt_topic_pub "/SmartParking/Client"
#define mqtt_topic_sub "/SmartParking/Master"
#define mqtt_user "ohkpjxcf"
#define mqtt_pwd "_3KWZeUTV7qe"
#define led 16
const uint16_t mqtt_port = 11486;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int state[] = {0,0,0,0};
int count = 3;
const int nSlot = 3;

SSD1306  display(0x3c, 4, 5);

void setup() {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  Serial.begin(115200);
  setup_wifi();
  pinMode(led,OUTPUT);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  display.clear();
  display.drawString(9,10, "Connected Wifi");
  display.display();
  delay(3000);
  display.clear();
  display.drawString(0,0, "Empty slot: "+(String)count);
  display.setFont(ArialMT_Plain_10);
  display.drawString(54,16, "MAP");
  display.drawHorizontalLine(0, 23, 50);
  display.drawHorizontalLine(80, 23, 50);
  display.drawRect(20, 30, 27, 34);
  display.drawRect(52, 30, 27, 34);
  display.drawRect(84, 30, 27, 34);
  //display.fillRect(55, 33, 21, 28);
  display.display();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String s="";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    s+=(char)payload[i];
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  int id = data[String("SlotID")];
  int st = data[String("SlotState")];
  Serial.print(id);
  snprintf (msg, 75, "ok %ld", id);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(mqtt_topic_pub, msg);
  state[id] = st;
  count=0;
  for (int i=1;i<=nSlot;i++)
    if (state[i]==0)
      count++;
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,0, "Empty slot: "+(String)count);
  display.setFont(ArialMT_Plain_10);
  display.drawString(54,16, "MAP");
  display.drawHorizontalLine(0, 23, 50);
  display.drawHorizontalLine(80, 23, 50);
  display.drawRect(20, 30, 27, 34);
  display.drawRect(52, 30, 27, 34);
  display.drawRect(84, 30, 27, 34);
  for (int i=1;i<=nSlot;i++){
    if (state[i]==1)
      display.fillRect(23+(i-1)*32, 33, 21, 28);
  }
  //display.fillRect(55, 33, 21, 28);
  display.display();
  //delay(3000);
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Server",mqtt_user, mqtt_pwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(mqtt_topic_pub, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 2000) {
    //Serial.println(count);
    //snprintf (msg, 75, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    //Serial.println(msg);
    //client.publish(mqtt_topic_pub, msg);
  }
}

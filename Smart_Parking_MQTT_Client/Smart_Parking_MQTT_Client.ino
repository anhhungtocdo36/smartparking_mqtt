#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// Update these with values suitable for your network.
#define ssid "SolarBKSES Guest"
#define password ""
#define mqtt_server "m14.cloudmqtt.com"
#define mqtt_topic_pub "/SmartParking/Master"
#define mqtt_topic_sub "/SmartParking/Client"
#define mqtt_user "ohkpjxcf"
#define mqtt_pwd "_3KWZeUTV7qe"
#define led 16
#define echoPin 13 // Echo Pin
#define trigPin 12 // Trigger Pin

#define len 35 

const int SlotID = 1;
int SlotState = 0;
bool flag = 0;

const uint16_t mqtt_port = 11486;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

StaticJsonBuffer<200> bf;
JsonObject &data = bf.createObject();

void setup() {
  Serial.begin(115200);
  setup_wifi();
  pinMode(led,OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
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
  String s = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    s+=(char)payload[i];
  }
  if (s==("ok "+(String)SlotID)){
    if (flag == 1)
      flag = 0;
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Client1",mqtt_user, mqtt_pwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(mqtt_topic_pub, "ESP connected");
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

long checkDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  long distance = duration / 58.2;
  Serial.println(distance);
  delay(50);
  return distance;
}

void UpdateState(){
  data["SlotID"] = SlotID;
  data["SlotState"] = SlotState;
  String s = "";
  data.printTo(s);
  for (int i=0;i<s.length();i++)
    msg[i] = s[i];
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(mqtt_topic_pub, msg);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (flag == 1){
    //flag = 0;
    UpdateState();
  }
  if(checkDistance() <= len){
    int count = 0;
    while (checkDistance()<=len){
      count++;
      if (count>10) break;
    }
    if (count>10){
      Serial.println(count);
      if (SlotState == 0){
        flag = 1;
        SlotState = 1;
      }  
    }
  }
  if(checkDistance() > len){
    int count = 0;
    while (checkDistance() > len){
      count++;
      if (count>10) break;
    }
    if (count>10){
      Serial.println(count);
      if (SlotState == 1){
        flag = 1;
        SlotState = 0;
      }  
    }
  }
}

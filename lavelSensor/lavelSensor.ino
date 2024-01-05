
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "IT_NW12";
const char* password = "123456788";

#define mqtt_server "192.168.31.10"
#define mqtt_port 1883
#define mqtt_user "project"
#define mqtt_password "123456788"

String id = "11";
String topicControl = "tabwater/device/" + id + "/control";
String topicStatus = "tabwater/device/" + id + "/alive";
String topicData = "tabwater/device/" + id + "/data";
#include <Ticker.h>
Ticker ticker;

#define BUILTIN_LED 2

WiFiClient espClient;
PubSubClient client(espClient);


void tick() {
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(14, OUTPUT);
  ticker.attach(0.8, tick);

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(id.c_str());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  ticker.detach();
  digitalWrite(BUILTIN_LED, LOW);

}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    digitalWrite(14,HIGH);
    WiFi.disconnect();
    WiFi.reconnect();
  }

    if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(id.c_str(), mqtt_user, mqtt_password, topicStatus.c_str(), 1, true, "offline"))  {
      Serial.println("connected");
    digitalWrite(14,LOW);
      client.publish(topicStatus.c_str(), "online", true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    digitalWrite(14,HIGH);
      delay(5000);
      ESP.restart();
    }
  }
  client.loop();

int reading = digitalRead(15);
String readingAsString = String(reading);
Serial.print(readingAsString);
client.publish(topicData.c_str(), readingAsString.c_str());

  delay(1000);


}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  int i = 0;
  while (i < length) msg += (char)payload[i++];

}

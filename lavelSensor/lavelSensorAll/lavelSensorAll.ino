
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "IT_NW12";
const char* password = "123456788";

StaticJsonDocument<500> doc;

#define mqtt_server "192.168.31.70"
#define mqtt_port 1883
#define mqtt_user "project"
#define mqtt_password "123456788"

String id = "10";
String topicControl = "tabwater/device/" + id + "/control";
String topicStatus = "tabwater/device/" + id + "/alive";
String topicData = "tabwater/device/" + id + "/data";
#include <Ticker.h>
Ticker ticker;

#define BUILTIN_LED 2

WiFiClient espClient;
PubSubClient client(espClient);


// PinMode input sensor
int untreated_water_day = 33;    //น้ำดิบกลางวัน
int untreated_water_night = 32;  //น้ำดิบกลางคืน
int tower_water_day = 35;        //หอสูงกลางวัน
int tower_water_night = 34;      //หอสูงกลางคืน



void tick() {
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(untreated_water_day, INPUT);
  pinMode(untreated_water_night, INPUT);
  pinMode(tower_water_day, INPUT);
  pinMode(tower_water_night, INPUT);
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
    digitalWrite(14, HIGH);
    WiFi.disconnect();
    WiFi.reconnect();
  }

  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(id.c_str(), mqtt_user, mqtt_password, topicStatus.c_str(), 1, true, "offline")) {
      Serial.println("connected");
      digitalWrite(14, LOW);
      client.publish(topicStatus.c_str(), "online", true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(14, HIGH);
      delay(5000);
      ESP.restart();
    }
  }
  client.loop();

  int untreated_water_day_reading = digitalRead(untreated_water_day);
  String untreated_water_day_readingAsString = String(untreated_water_day_reading);

  int untreated_water_nigth_reading = digitalRead(untreated_water_night);
  String untreated_water_nigth_readingAsString = String(untreated_water_nigth_reading);


  int tower_water_day_reading = digitalRead(tower_water_day);
  String tower_water_day_readingAsString = String(tower_water_day_reading);

  int tower_water_nigth_reading = digitalRead(tower_water_night);
  String tower_water_nigth_readingAsString = String(tower_water_nigth_reading);

  doc["untreated_water_day"] = untreated_water_day_readingAsString;
  doc["untreated_water_nigth"] = untreated_water_nigth_readingAsString;
  doc["tower_water_nigth"] = tower_water_nigth_readingAsString;
  doc["tower_water_day"] = tower_water_day_readingAsString;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
    client.publish(topicData.c_str(), buffer, n);
    doc.clear();

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

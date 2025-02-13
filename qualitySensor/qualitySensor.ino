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

String id = "15";
String topicControl = "tabwater/device/" + id + "/control";
String topicStatus = "tabwater/device/" + id + "/alive";
String topicData = "tabwater/device/" + id + "/data";
#include <Ticker.h>
Ticker ticker;

#define BUILTIN_LED 2

WiFiClient espClient;
PubSubClient client(espClient);


// PinMode input sensor
int before = 32;  // ก่อนกรอง
int after = 33;   // หลังกรอง
int flow = 35;    // ปริมาณ

float ntu_before;
float ntu_after;
float flow_data;

#define FLOW_PIN 35
#define FLOW_FACTOR (1.0 / 7.5)

unsigned long volume = 0;
unsigned long prevTime = 0;
uint32_t pulse_count = 0;

void ICACHE_RAM_ATTR on_trigger_handle() {
  pulse_count++;
}



void tick() {
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(before, INPUT);
  pinMode(after, INPUT);
  pinMode(flow, INPUT);
    pinMode(FLOW_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), on_trigger_handle, RISING);
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

  int sensorValue_before = analogRead(before);
  float voltage_before = sensorValue_before * (5.0 / 1024.0);
  ntu_before = -1120.4 * voltage_before * voltage_before + 5742.3 * voltage_before - 4352.9;

  int sensorValue_after = analogRead(after);
  float voltage_after = sensorValue_after * (5.0 / 1024.0);
  ntu_after = -1120.4 * voltage_after * voltage_after + 5742.3 * voltage_after - 4352.9;

    if ((millis() - prevTime) >= 1000) {
    uint32_t end_pulse_count = pulse_count;
    float flow = end_pulse_count * FLOW_FACTOR;  // 7.5 Hz = 1 L/min
    unsigned int flowMilliLitres = (flow / 60) * 1000;
    volume += flowMilliLitres;

    Serial.printf("Flow = %.02f L/min\n", flow);
    Serial.printf("Volume = %.02f ml\n", volume);
    doc["flowRate"] = String(flow);

    pulse_count = 0;  // Reset pulse count for the next measurement
    prevTime = millis();
  }

  doc["ntu_before"] = ntu_before;
  doc["ntu_after"] = ntu_after;

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
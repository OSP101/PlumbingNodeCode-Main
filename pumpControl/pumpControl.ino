#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include <Filters.h>

StaticJsonDocument<500> doc;
StaticJsonDocument<500> docStatus;

#if defined(ESP32)
PZEM004Tv30 pzem(Serial2, 16, 17);
#else
PZEM004Tv30 pzem(Serial2);
#endif

const char* ssid = "IT_NW12";
const char* password = "123456788";

#define mqtt_server "192.168.31.70"
#define mqtt_port 1883
#define mqtt_user "project"
#define mqtt_password "123456788"

String id = "3";
String topicControl = "tabwater/device/" + id + "/control";
String topicStatus = "tabwater/device/" + id + "/alive";
String topicData = "tabwater/device/" + id + "/data";
#include <Ticker.h>
Ticker ticker;

#define BUILTIN_LED 2
#define ZMPT101B 32          //ZMPT101B analog pin
#define PUMP_CONTROL_PIN 33  // กำหนดพินสำหรับรับคำสั่งควบคุมปั๊ม

int Current_Time = 0, Previous_Time = 0, Period = 2000; 

float testFrequency = 50;
float windowLength = 100 / testFrequency;

int RawValue = 0;

float intercept = 0; 
float slope = 1;      
float Volts_TRMS;     

float voltage;
float current;
float power;
float energy;
float frequency;
float pf;


RunningStatistics inputStats;

WiFiClient espClient;
PubSubClient client(espClient);



void tick() {
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void setup() {

  Serial.begin(115200);
  delay(10);
  pinMode(5, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(PUMP_CONTROL_PIN, INPUT);
  ticker.attach(0.8, tick);
  inputStats.setWindowSecs(windowLength);
  Serial.println();

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
  digitalWrite(5, HIGH);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
  }

  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(id.c_str(), mqtt_user, mqtt_password, topicStatus.c_str(), 1, true, "offline")) {
      Serial.println("connected");
      client.subscribe(topicControl.c_str());
      client.publish(topicStatus.c_str(), "online", true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      ESP.restart();
    }
  }


  if (WiFi.status() != WL_CONNECTED || !client.connected()) {
    int controlSignal = digitalRead(PUMP_CONTROL_PIN);
    digitalWrite(19, controlSignal);
  }

  client.loop();

  void callback(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    int i = 0;
    while (i < length) msg += (char)payload[i++];

    if (String(topic) == topicControl) {
      if (msg == "close") {
        digitalWrite(19, LOW);
        Serial.println(msg);
        doc["status"] = msg;

        return;
      } else if (msg == "open") {
        digitalWrite(19, HIGH);
        Serial.println(msg);
        doc["status"] = msg;

        return;
      }else if (msg == "auto") {
        digitalWrite(19, HIGH);
        Serial.println(msg);
        doc["status"] = msg;

        return;
      }
    }
  }

  Volts_TRMS = ReadVoltage();

  Current_Time = millis();

  if (Current_Time - Previous_Time >= Period) {
    Serial.print(F("\nSending Value "));
    Serial.print(Volts_TRMS);
    Serial.print("...");
    Serial.println(F("OK!"));


    Previous_Time = Current_Time;

    ReadVolMain();
    doc["overload"] = Volts_TRMS;
    doc["voltage"] = voltage;
    doc["current"] = current;
    doc["power"] = power;
    doc["energy"] = energy;
    doc["frequency"] = frequency;
    doc["pf"] = pf;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(topicData.c_str(), buffer, n);
    doc.clear();
  }
}

void ReadVolMain() {
  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();
}


float ReadVoltage() {

  RawValue = analogRead(ZMPT101B);  // read the analog in value:
  inputStats.input(RawValue);       // log to Stats function
  Volts_TRMS = inputStats.sigma() * slope + intercept;
  // Volts_TRMS = Volts_TRMS*0.979;

  return Volts_TRMS;
}

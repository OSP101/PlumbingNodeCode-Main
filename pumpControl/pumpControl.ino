/* This code works with ESP8266 12E and ZMPT101B AC voltage sensor
* It can measure the TRMS of Any voltage up to 250 VAC 50/60Hz and send the values to Adafruit MQTT
* Refer to www.SurtrTech.com for more details
*/

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

const char* ssid = "IT_NW12_2";
const char* password = "123456788";

#define mqtt_server "192.168.31.10"
#define mqtt_port 1883
#define mqtt_user "project"
#define mqtt_password "123456788"

String id = "2";
String topicControl = "tabwater/device/" + id + "/control";
String topicStatus = "tabwater/device/" + id + "/alive";
String topicData = "tabwater/device/" + id + "/data";
#include <Ticker.h>
Ticker ticker;

#define BUILTIN_LED 2
#define ZMPT101B 32 //ZMPT101B analog pin


int Current_Time=0, Previous_Time=0, Period=2000; //We send a value every 10s, the maximum is 30 value per minute

float testFrequency = 50; // test signal frequency (Hz)
float windowLength = 100/testFrequency; // how long to average the signal, for statistist

int RawValue = 0;

float intercept = 0; // to be adjusted based on calibration testing
float slope = 1; // to be adjusted based on calibration testing
float Volts_TRMS; // estimated actual voltage in Volts

  float voltage ;
  float current ;
  float power;
  float energy; 
  float frequency;
  float pf;

//For more details about the intercept and slope check the Base code at SurtrTech

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
  ticker.attach(0.8, tick);
inputStats.setWindowSecs( windowLength );
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

  client.loop();
  // delay(500);

Volts_TRMS=ReadVoltage(); //We read the voltage, normally the function returns the Volts_TRMS value,you can do it directly in the Test.publish
//But this is the syntax that worked for me, return a value to it self :D

Current_Time=millis();

if(Current_Time - Previous_Time >= Period){ //Every period we send the value to the service provider
Serial.print(F("\nSending Value ")); //The value is shown on the Serial monitor as well
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

void ReadVolMain(){
  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();

}


float ReadVoltage(){

RawValue = analogRead(ZMPT101B); // read the analog in value:
inputStats.input(RawValue); // log to Stats function
Volts_TRMS = inputStats.sigma()* slope + intercept;
// Volts_TRMS = Volts_TRMS*0.979;

return Volts_TRMS;

}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  int i = 0;
  while (i < length) msg += (char)payload[i++];

  if (String(topic) == topicControl) {
    if (msg == "off") {
      digitalWrite(19, LOW);
      Serial.println(msg);
      return;
    } else if (msg == "on") {
      digitalWrite(19, HIGH);
      Serial.println(msg);
      return;
    }
  }
}
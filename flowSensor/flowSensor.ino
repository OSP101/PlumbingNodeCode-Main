// #define FLOW_PIN 35
// #define FLOW_FACTOR (1.0 / 7.5)

// unsigned long volume;
// unsigned long currentMillis;
// unsigned long prevTime;
// uint32_t pulse_count = 0;
// void ICACHE_RAM_ATTR on_trigger_handle() {
//   pulse_count++;
// }


// void setup() {
//   Serial.begin(115200);

//   pinMode(FLOW_PIN, INPUT);
//   attachInterrupt(digitalPinToInterrupt(FLOW_PIN), on_trigger_handle, RISING);
// }

// void loop() {
//   pulse_count = 0;
//   currentMillis = millis();
//   delay(1000);
//   if ((millis() - prevTime) > 1000) {
//     uint32_t end_pulse_count = pulse_count;
//     float flow = end_pulse_count * FLOW_FACTOR;  // 7.5 Hz = 1 L/min
//     unsigned int flowMilliLitres = (flow / 60) * 1000;
//     volume += flowMilliLitres;
//     //
//      Serial.printf("Flow = %.02f L/min\n", flow);
//      Serial.printf("Volume = %.02f ml\n", volume);
//     // doc["flowRate"] = String(flow);
//     // doc["volume"] = String(volume);

//     prevTime = millis();

//     // char buffer[256];
//     // size_t n = serializeJson(doc, buffer);
//     // client.publish(topicData.c_str(), buffer, n);
//     // doc.clear();
//   }
// }

#define FLOW_PIN 35
#define FLOW_FACTOR (1.0 / 7.5)

unsigned long volume = 0;
unsigned long prevTime = 0;
uint32_t pulse_count = 0;

void ICACHE_RAM_ATTR on_trigger_handle() {
  pulse_count++;
}

void setup() {
  Serial.begin(115200);
  pinMode(FLOW_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), on_trigger_handle, RISING);
}

void loop() {
  if ((millis() - prevTime) >= 1000) {
    uint32_t end_pulse_count = pulse_count;
    float flow = end_pulse_count * FLOW_FACTOR;  // 7.5 Hz = 1 L/min
    unsigned int flowMilliLitres = (flow / 60) * 1000;
    volume += flowMilliLitres;

    Serial.printf("Flow = %.02f L/min\n", flow);
    Serial.printf("Volume = %.02f ml\n", volume);

    pulse_count = 0;  // Reset pulse count for the next measurement
    prevTime = millis();
  }
}
